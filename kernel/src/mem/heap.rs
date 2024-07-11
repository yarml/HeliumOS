use core::sync::atomic::{AtomicUsize, Ordering};

use linked_list_allocator::LockedHeap;
use spin::Mutex;
use x86_64::{
  structures::paging::{Page, PageTableFlags, Size4KiB},
  VirtAddr,
};

use crate::{dev::framebuffer::debug_set_pixel, mem, println, proc::apic};

use super::{valloc, PAGE_SIZE};

pub const START: Page<Size4KiB> = unsafe {
  Page::from_start_address_unchecked(VirtAddr::new_truncate(0xFFFF808000000000))
};
pub const SIZE: usize = 2 * 1024 * 1024 * 1024; // Maximum size of 2G
const FRAG_SIZE: usize = 32 * 1024; // 32K first time we need to expand

static EXPAND_COUNT: AtomicUsize = AtomicUsize::new(0);
static HEAP_HEAD: Mutex<u64> = Mutex::new(START.start_address().as_u64());

pub(in crate::mem) fn init() {
  // The following is a war crime. I am relying on the PageFault interrupt to
  // do memory allocation because I am lazy to write my own allocator that works
  // FIXME:
  // Well, the war crime came back to bite me, since each core allocates
  // 4K of heap for their NMI & DF stacks combined, any number of cores
  // higher than 8 causes the kernel to crash by double PageFault.
  // I will move the NMI and DF stacks away from the heap, but I want to keep
  // it documented here what can happen with this way of handling heap expansion
  // Basically, I should just avoid using the heap before late_start
  valloc(START, FRAG_SIZE / PAGE_SIZE, PageTableFlags::WRITABLE);
  unsafe {
    GLOB_ALLOCATOR
      .lock()
      .init(START.start_address().as_mut_ptr::<u8>(), SIZE)
  }
}

#[global_allocator]
static GLOB_ALLOCATOR: LockedHeap = LockedHeap::empty();

pub fn expand(adr: VirtAddr) {
  let mut heap_head = HEAP_HEAD.lock();
  debug_set_pixel(100, 110, (0, 255, 0).into());
  if adr.as_u64() < *heap_head {
    // There was a race condition, some other core already mapped
    // this area of the heap now, should be good to go
    return;
  }
  let heap_end = START.start_address() + SIZE as u64;
  let adr = Page::<Size4KiB>::containing_address(adr).start_address();
  let left = (heap_end - START.start_address()) as usize;
  let max_expand_amount =
    (EXPAND_COUNT.fetch_add(1, Ordering::SeqCst) + 1) * FRAG_SIZE;
  let expand = if left > max_expand_amount {
    max_expand_amount
  } else {
    left
  };

  println!(
    "[Proc {}] Expanding kernel heap by {} bytes from {:?}",
    apic::id(),
    expand,
    adr.as_ptr::<()>()
  );
  debug_set_pixel(100, 110, (255, 255, 0).into());
  valloc(
    Page::from_start_address(adr).unwrap(),
    expand / mem::PAGE_SIZE,
    PageTableFlags::WRITABLE,
  );
  *heap_head += expand as u64;
  debug_set_pixel(100, 110, (0, 0, 0).into());
}

pub fn is_heap(adr: VirtAddr) -> bool {
  let heap_end = START.start_address() + SIZE as u64;
  START.start_address() <= adr && adr < heap_end
}
