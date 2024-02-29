use x86_64::{
  structures::paging::{Page, PageTableFlags, PhysFrame, Size4KiB},
  PhysAddr, VirtAddr,
};

use crate::{bootboot::bootboot, mem::vmap};

const INITRD_START: Page<Size4KiB> = unsafe {
  Page::from_start_address_unchecked(VirtAddr::new_truncate(0xFFFF810000000000))
};

pub fn init() {
  let bootboot = bootboot();
  let initrd_frame = PhysFrame::<Size4KiB>::from_start_address(PhysAddr::new(
    bootboot.initrd_ptr,
  ))
  .unwrap();

  vmap(
    INITRD_START,
    initrd_frame,
    bootboot.initrd_size as usize,
    PageTableFlags::empty(),
  )
  .unwrap();
}
