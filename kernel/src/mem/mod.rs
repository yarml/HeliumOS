pub mod early_heap;
#[allow(dead_code)]
pub mod gdt;
pub mod heap;
mod phys;
pub mod virt;

use self::{
  phys::PHYS_FRAME_ALLOCATOR,
  virt::mapper::{KernelMapError, MAPPER},
};
use core::mem;
use x86_64::{
  align_up,
  structures::paging::{
    FrameAllocator, FrameDeallocator, Page, PageTableFlags, PhysFrame, Size4KiB,
  },
  VirtAddr,
};

pub const PAGE_SIZE: usize = 0x1000;

pub fn init() {
  gdt::basic_init();
  early_heap::init();
  phys::init();
  virt::init();
  heap::init();
}

pub fn palloc() -> Option<PhysFrame<Size4KiB>> {
  let mut allocator = PHYS_FRAME_ALLOCATOR.write();
  let result = allocator.allocate_frame();
  result
}
pub fn pfree(frame: PhysFrame<Size4KiB>) {
  unsafe { PHYS_FRAME_ALLOCATOR.write().deallocate_frame(frame) }
}
pub fn pfree_all(frames: &[PhysFrame<Size4KiB>]) {
  for frame in frames {
    pfree(*frame);
  }
}

pub fn vmap(
  page: Page<Size4KiB>,
  frame: PhysFrame<Size4KiB>,
  size: usize,
  flags: PageTableFlags,
) -> Result<(), KernelMapError> {
  MAPPER.write().map(page, frame, size, flags)
}
pub fn vmap_many(
  page: Page<Size4KiB>,
  frames: &[PhysFrame<Size4KiB>],
  flags: PageTableFlags,
) {
  for (i, frame) in frames.iter().enumerate() {
    vmap(page + i as u64, *frame, PAGE_SIZE, flags)
      .expect("Failed while mapping many")
  }
}

pub fn valloc(page: Page<Size4KiB>, n: usize, flags: PageTableFlags) {
  for off in 0..n {
    let frame = match palloc() {
      Some(frame) => frame,
      None => panic!("Could not allocate physical frame"),
    };
    let page =
      Page::from_start_address(page.start_address() + (off * PAGE_SIZE) as u64)
        .unwrap();
    vmap(page, frame, PAGE_SIZE, flags).expect("Could not map allocated frame");
  }
}

pub fn valloc_size(page: Page<Size4KiB>, nbytes: usize, flags: PageTableFlags) {
  let pgn = align_up(nbytes as u64, PAGE_SIZE as u64) as usize / PAGE_SIZE;
  valloc(page, pgn, flags);
}

pub fn valloc_ktable<T>(vadr: VirtAddr, n: usize) {
  valloc_size(
    Page::from_start_address(vadr).unwrap(),
    n * mem::size_of::<T>(),
    PageTableFlags::WRITABLE,
  );
}
