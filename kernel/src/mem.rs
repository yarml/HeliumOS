//! Temporary implementation to satistfy linker requirements

mod phys;
mod size;

pub use phys::PhysAddr;
pub use size::MemorySize;

use core::alloc::GlobalAlloc;

struct NoAlloc;

unsafe impl GlobalAlloc for NoAlloc {
  unsafe fn alloc(&self, _layout: core::alloc::Layout) -> *mut u8 {
    panic!()
  }

  unsafe fn dealloc(&self, _ptr: *mut u8, _layout: core::alloc::Layout) {
    panic!()
  }
}

#[global_allocator]
static GLOBALLOC: NoAlloc = NoAlloc;

pub fn init() {
  phys::init();
}
