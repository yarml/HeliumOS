use super::KVMSPACE;
use crate::mem::{
  early_heap::EarlyAllocator, phys::PHYS_FRAME_ALLOCATOR, PAGE_SIZE,
};
use alloc::{boxed::Box, slice};
use core::mem;
use spin::RwLock;
use x86_64::{
  registers::control::Cr3,
  structures::paging::{
    mapper::MapperFlush, page_table::PageTableEntry, FrameAllocator, Page,
    PageTable, PageTableFlags, PhysFrame, Size4KiB,
  },
  PhysAddr, VirtAddr,
};

const START_ADDR: VirtAddr = KVMSPACE;
const P1PAGE_COUNT: usize = 2048;
const P2PAGE_COUNT: usize = P1PAGE_COUNT / 512;
// const SIZE: usize = P1PAGE_COUNT * PAGE_SIZE;

pub fn init() {
  let (p4_frame, _) = Cr3::read();
  let p4 =
    unsafe { (p4_frame.start_address().as_u64() as *mut PageTable).as_mut() }
      .unwrap();
  let p4_entry = &mut p4[START_ADDR.p4_index()];
  if p4_entry.is_unused() {
    alloc_substruct(p4_entry);
  }

  let p3 = unsafe {
    (p4_entry.frame().unwrap().start_address().as_u64() as *mut PageTable)
      .as_mut()
  }
  .unwrap();

  let p3_entry = &mut p3[START_ADDR.p3_index()];

  if p3_entry.is_unused() {
    alloc_substruct(p3_entry);
  }

  let p2_frame = p3_entry.frame().unwrap();
  let p2 =
    unsafe { (p2_frame.start_address().as_u64() as *mut PageTable).as_mut() }
      .unwrap();

  {
    for i in 0..P2PAGE_COUNT {
      let p2_entry = &mut p2[i];
      if !p2_entry.is_unused() {
        panic!("VCache target P2 page entries found in invalid state");
      }

      alloc_substruct(p2_entry);
    }
  }

  let p1_frame = p2[0].frame().unwrap();
  let p1 =
    unsafe { (p1_frame.start_address().as_u64() as *mut PageTable).as_mut() }
      .unwrap();

  let vcache_state = VCacheState::new();
  let mut vcache = VCache {
    p1: slice::from_mut(p1),
    state: vcache_state,
  };

  // FIXME: In the following we are making two assumptions about VCache at this state
  // 1- Pages returned will be sequential
  // 2- Will only need to access the first PT
  // Both of these assumption are not part of the specification of the VCache
  // meaning, if the implementation changes, this might break!
  for i in 0..P2PAGE_COUNT {
    let p1_virtaddr = match vcache.map(
      p1_frame.start_address() + i * 512 * mem::size_of::<PageTableEntry>(),
    ) {
      Err(()) => panic!("Could not map VCache P1 table"),
      Ok(page) => page.start_address(),
    };

    // This check has to do with the FIXME above
    if p1_virtaddr != START_ADDR + PAGE_SIZE * i {
      panic!(
        "Time to fix this ugly relianceo n undefined VCache map() behavior"
      );
    }
  }

  let p1_virtadr = START_ADDR;

  let p1_ptr = p1_virtadr.as_u64() as *mut PageTable;
  let p1 = unsafe { slice::from_raw_parts_mut(p1_ptr, P2PAGE_COUNT) };
  // Now let's make a completely valid VCache
  let vcache = VCache::renew(vcache.state, p1);

  // Now we move this to be the global vcache
  let mut vcache_glob = VCACHE.write();
  *vcache_glob = Some(vcache);
}

fn alloc_substruct(entry: &mut PageTableEntry) {
  let mut allocator = PHYS_FRAME_ALLOCATOR.write();
  let frame = match allocator.allocate_frame() {
    None => panic!("Could not allocate memory while initializing VCache"),
    Some(frame) => frame,
  };

  // Clear newly allocated frame
  {
    let frame_ptr = frame.start_address().as_u64() as *mut u8;
    let frame_ref = unsafe { slice::from_raw_parts_mut(frame_ptr, PAGE_SIZE) };
    frame_ref.fill(0);
  }

  *entry = PageTableEntry::new();
  entry.set_frame(frame, PageTableFlags::WRITABLE | PageTableFlags::PRESENT);
}

static VCACHE: RwLock<Option<VCache>> = RwLock::new(None);

struct VCache<'a> {
  p1: &'a mut [PageTable],
  state: Box<VCacheState, EarlyAllocator>, // Using a box, otherwise this beast eats too much stack
}
struct VCacheState {
  p2_freecount: [u16; P2PAGE_COUNT],
  p2_lazycount: [u16; P2PAGE_COUNT],
  p1_age: [u16; P1PAGE_COUNT],
}

impl VCacheState {
  fn new() -> Box<Self, EarlyAllocator> {
    Box::new_in(
      Self {
        p2_freecount: [512; P2PAGE_COUNT],
        p2_lazycount: [0; P2PAGE_COUNT],
        p1_age: [0; P1PAGE_COUNT],
      },
      EarlyAllocator,
    )
  }
}

impl<'a> VCache<'a> {
  fn renew(
    state: Box<VCacheState, EarlyAllocator>,
    p1: &'a mut [PageTable],
  ) -> Self {
    Self { p1, state }
  }
  fn map(&mut self, phy_addr: PhysAddr) -> Result<Page, ()> {
    // First pass, look for lazy pages already pointing at this physical address
    for p2i in 0..P2PAGE_COUNT {
      let lazy_count = &mut self.state.p2_lazycount[p2i];

      if *lazy_count == 0 {
        continue;
      }

      for p1i in 0..512 {
        let p1_entry = &self.p1[p2i][p1i];
        let age = &mut self.state.p1_age[p2i * 512 + p1i];
        let frame = match p1_entry.frame() {
          Ok(frame) => frame,
          Err(_) => continue,
        };
        if !p1_entry.is_unused()
          && *age != 0
          && frame.start_address() == phy_addr
        {
          *age = 0;
          *lazy_count -= 1;
          let page = Page::<Size4KiB>::from_start_address(
            START_ADDR + PAGE_SIZE * (p2i * 512 + p1i),
          )
          .unwrap();
          return Ok(page);
        }
      }
    }

    // No good lazy page was found, look for a P2 table with at least 1 free page

    let mut lazy_pages_count = 0u16;

    let mut p2index: Option<usize> = None;
    let mut p1index: Option<usize> = None;

    for i in 0..P2PAGE_COUNT {
      if lazy_pages_count != 0 {
        lazy_pages_count = self.state.p2_lazycount[i];
      }
      if self.state.p2_freecount[i] != 0 {
        p2index = Some(i);
        break;
      }
    }

    if let None = p2index {
      if lazy_pages_count == 0 {
        // No free pages were found, and no lazy pages can be freed either
        return Err(());
      }

      // No free pages were found, but some lazy pages exist, allocation can still be done
      for p2i in 0..P2PAGE_COUNT {
        if self.state.p2_freecount[p2i] != 0 {
          let mut lazy_found_count = 0u16;
          p2index = Some(p2i);
          // Iterate through all P1 tables of this P2 entry until we found all the lazy
          // pages and marked them as free
          for p1i in 0..512 {
            if lazy_found_count >= lazy_pages_count {
              break;
            }

            let age = &mut self.state.p1_age[p2i * 512 + p1i];
            if *age != 0 {
              lazy_found_count += 1;
              *age = 0;
              let mut flags = self.p1[p2i][p1i].flags();
              flags.remove(PageTableFlags::PRESENT);
              self.p1[p2i][p1i].set_flags(flags);

              p1index = Some(p1i);
              // Don't bother invalidating the pages, they will be invalidated when remapped
            }
          }

          self.state.p2_freecount[p2i] = lazy_found_count;
          self.state.p2_lazycount[p2i] = 0;

          // We only clear 1 PDE, do not bother with the other ones as we don't need them now
          break;
        }
      }
    }

    let p2index = p2index.unwrap();
    self.state.p2_freecount[p2index] -= 1;

    // If we went through lazy page invalidation, we already have a target_p1e
    if let None = p1index {
      for p1i in 0..512 {
        let flags = self.p1[p2index][p1i].flags();
        if !flags.contains(PageTableFlags::PRESENT) {
          p1index = Some(p1i);
          break;
        }
      }

      // Mark the page as not lazy
      self.state.p1_age[p2index * 512 + p1index.unwrap()] = 0;
    }

    let p1index = p1index.unwrap();

    // Now we setup the P1 entry
    let target_p1e = &mut self.p1[p2index][p1index];
    *target_p1e = PageTableEntry::new();
    target_p1e.set_frame(
      PhysFrame::from_start_address(phy_addr).unwrap(),
      PageTableFlags::WRITABLE
        | PageTableFlags::GLOBAL
        | PageTableFlags::PRESENT,
    );

    let virt_addr = START_ADDR + PAGE_SIZE * (p2index * 512 + p1index);
    let page = Page::<Size4KiB>::from_start_address(virt_addr).unwrap();
    MapperFlush::new(page).flush();

    Ok(page)
  }
}
