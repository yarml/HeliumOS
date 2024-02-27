use core::slice;

use x86_64::{
  structures::paging::{
    page_table::PageTableEntry, FrameAllocator, PageTableFlags, PhysFrame,
  },
  VirtAddr,
};

use super::{
  phys::{self, PHYS_FRAME_ALLOCATOR},
  PAGE_SIZE,
};

pub mod mapper;
pub mod vcache;

pub const KVMSPACE: VirtAddr = VirtAddr::new_truncate(0xFFFF800000000000);

pub(in crate::mem) fn init() {
  vcache::init();
  mapper::init();
  phys::map_header();
}

fn alloc_substruct(
  entry: &mut PageTableEntry,
  flags: PageTableFlags,
) -> PhysFrame {
  let mut allocator = PHYS_FRAME_ALLOCATOR.write();
  let frame = match allocator.allocate_frame() {
    None => panic!("Could not allocate memory while initializing VCache"),
    Some(frame) => frame,
  };

  // Clear newly allocated frame
  {
    let frame_ptr = frame.start_address().as_u64() as *mut u8;
    let frame_ref = unsafe { slice::from_raw_parts_mut(frame_ptr, PAGE_SIZE) };
    frame_ref.fill(0);
  }

  *entry = PageTableEntry::new();
  entry.set_frame(frame, flags);

  frame
}
