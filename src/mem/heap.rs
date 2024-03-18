use core::{
  alloc::{GlobalAlloc, Layout},
  ops::Add,
  ptr::null_mut, slice,
};
use spin::RwLock;
use x86_64::{
  align_up,
  structures::paging::{Page, PageTableFlags, Size4KiB},
  VirtAddr,
};

use crate::{
  mem::{palloc, vmap, PAGE_SIZE},
  println,
};

const START: Page<Size4KiB> = unsafe {
  Page::from_start_address_unchecked(VirtAddr::new_truncate(0xFFFF808000000000))
};
const SIZE: usize = 2 * 1024 * 1024 * 1024; // Maximum size of 2G
const INIT_SIZE: usize = 32 * 1024; // Start with 32K

pub(in crate::mem) fn init() {
  let mut allocator = KERNEL_ALLOCATOR.write();
  let r = allocator.expand(INIT_SIZE);
  r.unwrap();
}

static KERNEL_ALLOCATOR: RwLock<KernelAllocator> =
  RwLock::new(KernelAllocator {
    size: 0,
    cursor: 0,
    start: START,
  });

// FIXME: Come on, bump allocator?? I can do better
struct KernelAllocator {
  size: usize,
  start: Page<Size4KiB>,
  cursor: usize,
}

impl KernelAllocator {
  fn expand(&mut self, delta_size: usize) -> Result<(), ()> {
    let size =
      self.size + align_up(delta_size as u64, PAGE_SIZE as u64) as usize;
    let num_pages = size / PAGE_SIZE;
    if size > SIZE {
      return Err(());
    }
    if size <= self.size {
      return Ok(());
    }

    // Allocate and map new pages
    let next_page = Page::<Size4KiB>::from_start_address(
      self.start.start_address() + self.size,
    )
    .unwrap();

    for i in 0..num_pages {
      let page = next_page.add(i as u64);
      let frame = match palloc() {
        Some(frame) => frame,
        None => return Err(()),
      };

      match vmap(page, frame, PAGE_SIZE, PageTableFlags::WRITABLE) {
        Err(_) => return Err(()),
        _ => {}
      };
    }

    return Ok(());
  }

  fn alloc(&mut self, layout: Layout) -> *mut u8 {
    if layout.align() > PAGE_SIZE {
      panic!("Unsupported alignment");
    }

    let cursor = align_up(
      self.start.start_address().as_u64() + self.cursor as u64,
      layout.align() as u64,
    ) as usize;
    let rel_cursor = cursor - self.start.start_address().as_u64() as usize;
    if rel_cursor + layout.size() > self.size {
      let diff = rel_cursor + layout.size() - self.size;
      let expand_size = if diff > INIT_SIZE { diff } else { INIT_SIZE };
      match self.expand(expand_size) {
        Err(_) => return null_mut(),
        _ => {}
      };
    }

    self.cursor = rel_cursor + layout.size();
    let ptr = cursor as *mut u8;
    println!(
      "Alloc => {:?}:{:?} (size={:x}, align={:x})",
      ptr,
      unsafe { ptr.add(layout.size()) },
      layout.size(),
      layout.align(),
    );
    let alloc = unsafe { slice::from_raw_parts_mut(ptr, layout.size()) };
    alloc.fill(0);
    assert_eq!(ptr as usize % layout.align(), 0);
    ptr
  }
}

struct GlobKernelAllocator;

unsafe impl GlobalAlloc for GlobKernelAllocator {
  unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
    KERNEL_ALLOCATOR.write().alloc(layout)
  }

  unsafe fn dealloc(&self, _ptr: *mut u8, _layout: Layout) {
    // 7adretek i7na benesta3mil al bump allocator, dah dealloc howa nafso al nnoop
  }
}

#[global_allocator]
static GLOB_ALLOCATOR: GlobKernelAllocator = GlobKernelAllocator;
