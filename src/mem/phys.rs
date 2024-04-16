use super::early_heap::EarlyAllocator;
use super::vmap;
use crate::bootboot::{bootboot, MMapEnt, MMapType, BootBoot};
use crate::mem::PAGE_SIZE;
use crate::println;
use alloc::vec::Vec;
use core::mem;
use spin::RwLock;
use x86_64::structures::paging::{
  FrameAllocator, FrameDeallocator, Page, PageTableFlags, PhysFrame, Size4KiB,
};
use x86_64::{align_up, PhysAddr, VirtAddr};

const PHYSHEADER_START: Page<Size4KiB> = unsafe {
  Page::from_start_address_unchecked(VirtAddr::new_truncate(0xFFFF804000000000))
};

#[derive(Clone, Copy)]
struct SegmentHeader {
  magic: u64,
  address: PhysAddr,
  size: usize,
}

impl SegmentHeader {
  const MAGIC: u64 = 0xA55AA55AA55AA55A;
  pub const fn new(address: PhysAddr, size: usize) -> Self {
    SegmentHeader {
      magic: Self::MAGIC,
      address,
      size,
    }
  }

  pub fn check(&self) {
    assert_eq!(self.magic, Self::MAGIC);
  }

  pub fn bitmap_size(&self) -> usize {
    (align_up((self.size / PAGE_SIZE) as u64, 64) / 8) as usize
  }
  pub fn bitmap_u64len(&self) -> usize {
    self.bitmap_size() / 8
  }

  #[allow(clippy::mut_from_ref)]
  pub fn bitmap(&self) -> &mut [u64] {
    let self_ptr = self as *const SegmentHeader;
    let bitmap_ptr = unsafe { self_ptr.add(1) } as *mut u64;
    unsafe { core::slice::from_raw_parts_mut(bitmap_ptr, self.bitmap_u64len()) }
  }

  pub fn in_range(&self, addr: PhysAddr) -> bool {
    self.address <= addr && addr < self.address + self.size as u64
  }
}

pub(in crate::mem) fn init() {
  // Just in case I forget
  assert_eq!(core::mem::size_of::<SegmentHeader>() % 8, 0);

  let bootboot = bootboot();

  let mmap_len = (bootboot.size as usize - mem::size_of::<BootBoot>())
    / mem::size_of::<MMapEnt>()
    + 1;

  let mut mmap_usable = Vec::new_in(EarlyAllocator);
  let mut header_total_size = 0usize;

  let mmap_start = &bootboot.mmap as *const MMapEnt;

  /* Insert the regions backward, so that the PMM would prefer allocating
  pages with higher physical adresses and leave the lower addresses for
  hardware that requires it */
  for i in (0..mmap_len).rev() {
    let mmap = unsafe { mmap_start.add(i).as_ref().unwrap() };
    println!(
      "mregion(type={:?}, phyadr={:?}, size={:x})",
      mmap.get_type(),
      mmap.phyadr(),
      mmap.size()
    );

    if mmap.get_type() == MMapType::Free && mmap.raw_size() >= PAGE_SIZE {
      let segment_header = SegmentHeader::new(mmap.phyadr(), mmap.size());
      header_total_size +=
        segment_header.bitmap_size() + core::mem::size_of::<SegmentHeader>();
      mmap_usable.push(segment_header);
    }
  }

  let mut pmm_header: Option<PhysAddr> = None;
  /* Search for a place for the PMM header */
  for segment in mmap_usable.iter_mut() {
    if segment.size
      >= align_up(header_total_size as u64, PAGE_SIZE as u64) as usize
    {
      pmm_header = Some(segment.address);
      segment.address += align_up(header_total_size as u64, PAGE_SIZE as u64);
      segment.size -=
        align_up(header_total_size as u64, PAGE_SIZE as u64) as usize;
      break;
    }
  }

  if pmm_header.is_none() {
    panic!("Could not find a place for the PMM header");
  }

  let pmm_header = pmm_header.unwrap();
  let mut header_cursor = 0usize;

  let mut segment_references = Vec::new_in(EarlyAllocator);
  /* Initialize PMM header & setup the segment header reference vector */
  for segment_tmp in mmap_usable {
    let segment_ptr =
      (pmm_header + header_cursor as u64).as_u64() as *mut SegmentHeader;
    let segment = unsafe { segment_ptr.as_mut().unwrap() };
    *segment = segment_tmp;
    let bitmap = segment.bitmap();
    bitmap.fill(0);
    if let Some(last) = bitmap.last_mut() {
      *last = u64::MAX << ((segment_tmp.size / PAGE_SIZE) % 64);
    } else {
      unreachable!()
    }

    header_cursor +=
      segment.bitmap_size() + core::mem::size_of::<SegmentHeader>();

    println!(
      "header(header_address={:?}, address={:?}, size={:x})",
      segment_ptr,
      segment_tmp.address,
      segment_tmp.size / PAGE_SIZE
    );
    segment_references.push(segment as &SegmentHeader);
  }

  println!(
    "pmm header(adr={:?}, offset={:x}, size={:x}",
    pmm_header, header_cursor, header_total_size
  );

  let mut physical_allocator = PHYS_FRAME_ALLOCATOR.write();
  physical_allocator.segments = segment_references;
  physical_allocator.size = header_total_size;
  physical_allocator.phyframe =
    PhysFrame::from_start_address(pmm_header).unwrap();
}

pub(super) fn map_header() {
  // Kernel mapper uses the physical frame allocator
  // Limit the scope of the lock to prevent a dead lock
  let (pmm_frame, pmm_size) = {
    let physallocator = PHYS_FRAME_ALLOCATOR.read();
    (physallocator.phyframe, physallocator.size)
  };
  vmap(
    PHYSHEADER_START,
    pmm_frame,
    pmm_size,
    PageTableFlags::WRITABLE,
  )
  .unwrap();
  let segment_offset = (PHYSHEADER_START.start_address().as_u64()
    - pmm_frame.start_address().as_u64()) as usize;

  let mut physallocator = PHYS_FRAME_ALLOCATOR.write();

  let offset_segment_iter = physallocator.segments.iter().map(|phys_segment| {
    let phys_segment = *phys_segment;
    let phys_segment_ptr = phys_segment as *const SegmentHeader as usize;
    let offset_segment_ptr = phys_segment_ptr + segment_offset;
    unsafe { (offset_segment_ptr as *const SegmentHeader).as_ref() }.unwrap()
  });
  let mut offset_segments_refs = Vec::new_in(EarlyAllocator);
  for offset_seg in offset_segment_iter {
    offset_segments_refs.push(offset_seg);
  }

  physallocator.segments = offset_segments_refs;
}

pub struct PhysicalFrameAllocator {
  segments: Vec<&'static SegmentHeader, EarlyAllocator>,
  size: usize,
  phyframe: PhysFrame<Size4KiB>,
}

impl PhysicalFrameAllocator {
  const fn new() -> Self {
    Self {
      segments: Vec::new_in(EarlyAllocator),
      size: 0,
      phyframe: unsafe {
        PhysFrame::from_start_address_unchecked(PhysAddr::zero())
      },
    }
  }
}

pub static PHYS_FRAME_ALLOCATOR: RwLock<PhysicalFrameAllocator> =
  RwLock::new(PhysicalFrameAllocator::new());

unsafe impl FrameAllocator<Size4KiB> for PhysicalFrameAllocator {
  fn allocate_frame(&mut self) -> Option<PhysFrame<Size4KiB>> {
    for segment in self.segments.iter() {
      segment.check();
      let bitmap = segment.bitmap();

      let (page_windex, control_word) = match bitmap
        .iter_mut()
        .enumerate()
        .find(|(_, control_word)| **control_word != u64::MAX)
      {
        Some(result) => result,
        None => continue,
      };

      let page_bitindex = control_word.trailing_ones() as usize; // Cannot be 64, hopefully
      assert_ne!(page_bitindex, 64);
      let page_index = 64 * page_windex + page_bitindex;
      *control_word |= 1 << page_bitindex;
      let frame_start = segment.address + (page_index * PAGE_SIZE) as u64;
      return Some(PhysFrame::containing_address(frame_start));
    }
    None
  }
}

impl FrameDeallocator<Size4KiB> for PhysicalFrameAllocator {
  unsafe fn deallocate_frame(&mut self, frame: PhysFrame<Size4KiB>) {
    // TODO: Stop being lazy and do binary search
    let segment = match self
      .segments
      .iter()
      .find(|seg| seg.in_range(frame.start_address()))
    {
      None => return, // What are we deallocating???
      Some(segment) => segment,
    };
    let bitmap = segment.bitmap();
    let frame_offset = (frame.start_address() - segment.address) as usize;
    let page_index = frame_offset / PAGE_SIZE;
    let page_windex = page_index / 64;
    let page_bitindex = page_index % 64;
    let control_word = &mut bitmap[page_windex];
    *control_word &= !(1 << page_bitindex);
  }
}
