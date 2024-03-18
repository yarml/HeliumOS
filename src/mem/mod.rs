use x86_64::structures::paging::{
  FrameAllocator, FrameDeallocator, Page, PageTableFlags, PhysFrame, Size4KiB,
};

use self::{
  phys::PHYS_FRAME_ALLOCATOR,
  virt::mapper::{KernelMapError, MAPPER},
};

pub mod early_heap;
#[allow(dead_code)]
mod gdt;
mod heap;
mod phys;
pub mod virt;

pub const PAGE_SIZE: usize = 0x1000;

pub fn init() {
  gdt::basic_init();
  early_heap::init();
  phys::init();
  virt::init();
  heap::init();
}

pub fn palloc() -> Option<PhysFrame<Size4KiB>> {
  PHYS_FRAME_ALLOCATOR.write().allocate_frame()
}
pub fn pfree(frame: PhysFrame<Size4KiB>) {
  unsafe { PHYS_FRAME_ALLOCATOR.write().deallocate_frame(frame) }
}

pub fn vmap(
  page: Page<Size4KiB>,
  frame: PhysFrame<Size4KiB>,
  size: usize,
  flags: PageTableFlags,
) -> Result<(), KernelMapError> {
  MAPPER.write().map(page, frame, size, flags)
}
