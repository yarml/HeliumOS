mod addr;
mod alloc;
mod frame;
mod internal;

pub use addr::PhysAddr;
pub use alloc::FrameAllocation;
pub use frame::Frame;

use crate::{bootboot::BootbootHeader, debug};

pub fn init() {
  let mmap = BootbootHeader::instance().mmap();
  for entry in mmap {
    debug!("{:?}", entry);
  }
}
