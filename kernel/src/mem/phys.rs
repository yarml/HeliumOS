mod addr;
mod alloc;
mod internal;

pub use addr::PhysAddr;
pub use alloc::FrameAllocation;

use crate::{bootboot::BootbootHeader, debug};

pub fn init() {
  let mmap = BootbootHeader::instance().mmap();
  for entry in mmap {
    debug!("{:?}", entry);
  }
}
