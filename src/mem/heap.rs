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
  // Well, the war crime came back to bite me, since each core allocates
  // 4K of heap for their NMI & DF stacks combined, any number of cores
  // higher than 8 causes the kernel to crash by double PageFault.
  // I will move the NMI and DF stacks away from the heap, but I want to keep
  // it documented here what can happen with this way of handling heap expansion
  // Another problem(serious) that I thought of is if two cores at the same
  // time get the heap expansion PageFault, the lock of valloc will still not
  // protect against the speed condition. The first maps the expanded area
  // to some physical memory, while the other maps it to another area, invalidating
  // any writes done by the first core. This latter issue is really critical
  valloc(START, FRAG_SIZE / PAGE_SIZE, PageTableFlags::WRITABLE);
  unsafe {
    GLOB_ALLOCATOR
      .lock()
      .init(START.start_address().as_mut_ptr::<u8>(), SIZE)
  }
}

#[global_allocator]
static GLOB_ALLOCATOR: LockedHeap = LockedHeap::empty();
