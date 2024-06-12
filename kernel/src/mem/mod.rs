use crate::{debug, dev::framebuffer::debug_set_pixel, println};

use self::{
  phys::PHYS_FRAME_ALLOCATOR,
  virt::mapper::{KernelMapError, MAPPER},
};
use core::mem;
use x86_64::{
  align_up, structures::paging::{
    FrameAllocator, FrameDeallocator, Page, PageTableFlags, PhysFrame, Size4KiB,
  }, VirtAddr
};

pub mod early_heap;
#[allow(dead_code)]
pub mod gdt;
pub mod heap;
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
  let mut allocator = PHYS_FRAME_ALLOCATOR.write();
  debug_set_pixel(100, 111, (0, 255, 0).into());
  let result = allocator.allocate_frame();
  debug_set_pixel(100, 111, (0, 0, 0).into());
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
  let mut total_palloc_time = 0;
  let mut total_vmap_time = 0;
  let start_time = debug::rdtsc();
  for off in 0..n {
    let palloc_start = debug::rdtsc();
    let frame = match palloc() {
      Some(frame) => frame,
      None => panic!("Could not allocate physical frame"),
    };
    let palloc_end = debug::rdtsc();
    let page =
      Page::from_start_address(page.start_address() + (off * PAGE_SIZE) as u64)
        .unwrap();
    debug_set_pixel(100, 112, (0, 0, 255).into());
    let vmap_start = debug::rdtsc();
    vmap(page, frame, PAGE_SIZE, flags).expect("Could not map allocated frame");
    let vmal_end = debug::rdtsc();
    debug_set_pixel(100, 112, (0, 0, 0).into());
    total_palloc_time += palloc_end - palloc_start;
    total_vmap_time += vmal_end - vmap_start;
  }
  let end_time = debug::rdtsc();
  println!(
    "Valloc took {} cycles, palloc took {} cycles of that ({}%), vmap took {} cycles of that ({}%)",
    end_time - start_time,
    total_palloc_time,
    total_palloc_time * 100 / (end_time - start_time),
    total_vmap_time,
    total_vmap_time * 100 / (end_time - start_time)
  );
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
