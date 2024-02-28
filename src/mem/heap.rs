use core::alloc::{GlobalAlloc, Layout};

pub(in crate::mem) fn init() {
}

struct Allocator;

unsafe impl GlobalAlloc for Allocator {
  unsafe fn alloc(&self, _layout: Layout) -> *mut u8 {
    todo!()
  }

  unsafe fn dealloc(&self, _ptr: *mut u8, _layout: Layout) {
    todo!()
  }
}

#[global_allocator]
static ALLOCATOR: Allocator = Allocator;
