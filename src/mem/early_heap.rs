use core::{
  alloc::{AllocError, Allocator, GlobalAlloc, Layout},
  ptr::NonNull,
};

use linked_list_allocator::LockedHeap;
use spin::Mutex;

const EARLY_HEAP_SIZE: usize = 128 * 1024;

static EARLY_HEAP: Mutex<[u8; EARLY_HEAP_SIZE]> =
  Mutex::new([0u8; EARLY_HEAP_SIZE]);

static EARLY_ALLOCATOR: LockedHeap = LockedHeap::empty();

pub fn init() {
  let mut early_heap_guard = EARLY_HEAP.lock();
  unsafe {
    EARLY_ALLOCATOR
      .lock()
      .init(early_heap_guard.as_mut_ptr(), early_heap_guard.len())
  }
}

pub struct EarlyAllocator;

unsafe impl Allocator for EarlyAllocator {
  fn allocate(
    &self,
    layout: core::alloc::Layout,
  ) -> Result<NonNull<[u8]>, AllocError> {
    match layout.size() {
      0 => Ok(NonNull::slice_from_raw_parts(layout.dangling(), 0)),
      size => {
        let raw_ptr = unsafe { EARLY_ALLOCATOR.alloc(layout) };
        let ptr = NonNull::new(raw_ptr).ok_or(AllocError)?;
        Ok(NonNull::slice_from_raw_parts(ptr, size))
      }
    }
  }

  unsafe fn deallocate(&self, ptr: NonNull<u8>, layout: Layout) {
    if layout.size() != 0 {
      EARLY_ALLOCATOR.dealloc(ptr.as_ptr(), layout)
    }
  }
}

// Temporary DummyAllocator for a global_allocator, this will be replaced later in development
struct DummyAllocator;

unsafe impl GlobalAlloc for DummyAllocator {
  unsafe fn alloc(&self, _layout: Layout) -> *mut u8 {
    panic!("Using dummy allocator")
  }

  unsafe fn dealloc(&self, _ptr: *mut u8, _layout: Layout) {
    panic!("Using dummt allocator")
  }
}

#[global_allocator]
static DUMMY_ALLOCATOR: DummyAllocator = DummyAllocator;
