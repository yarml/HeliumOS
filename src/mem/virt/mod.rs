use core::slice;

use x86_64::{
  structures::paging::{page_table::PageTableEntry, PageTableFlags, PhysFrame},
  VirtAddr,
};

use super::{
  palloc,
  phys::{self},
  PAGE_SIZE,
};

pub mod mapper;
pub mod vcache;

pub const KVMSPACE: VirtAddr = VirtAddr::new_truncate(0xFFFF800000000000);

pub(in crate::mem) fn init() {
  vcache::init();
  mapper::init();
  phys::map_header();
  mapper::remove_identity();
}

fn alloc_substruct(
  entry: &mut PageTableEntry,
  flags: PageTableFlags,
  clear_inplace: bool,
) -> PhysFrame {
  let frame = match palloc() {
    None => panic!("Could not allocate memory while initializing VCache"),
    Some(frame) => frame,
  };

  if clear_inplace {
    // Clear newly allocated frame
    let frame_ptr = frame.start_address().as_u64() as *mut u8;
    let frame_ref = unsafe { slice::from_raw_parts_mut(frame_ptr, PAGE_SIZE) };
    frame_ref.fill(0);
  }

  *entry = PageTableEntry::new();
  entry.set_frame(frame, flags);

  frame
}
