use linked_list_allocator::LockedHeap;
use x86_64::{
  structures::paging::{Page, PageTableFlags, Size4KiB},
  VirtAddr,
};

use super::{valloc, PAGE_SIZE};

pub const START: Page<Size4KiB> = unsafe {
  Page::from_start_address_unchecked(VirtAddr::new_truncate(0xFFFF808000000000))
};
pub const SIZE: usize = 2 * 1024 * 1024 * 1024; // Maximum size of 2G
pub const FRAG_SIZE: usize = 32 * 1024; // 32K each time we need more memory

pub(in crate::mem) fn init() {
  // The following is a war crime. I am relying on the PageFault interrupt to
  // do memory allocation because I am lazy to write my own allocator that works
  // FIXME:
  valloc(START, FRAG_SIZE / PAGE_SIZE, PageTableFlags::WRITABLE);
  unsafe {
    GLOB_ALLOCATOR
      .lock()
      .init(START.start_address().as_mut_ptr::<u8>(), SIZE)
  }
}

#[global_allocator]
static GLOB_ALLOCATOR: LockedHeap = LockedHeap::empty();
