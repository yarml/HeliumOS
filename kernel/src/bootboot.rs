mod framebuffer;
mod header;
mod mmap;

use core::ffi;

pub use header::Header as BootbootHeader;
pub use mmap::{Entry as MmapEntry, Type as MmapType};

extern "C" {
  static bootboot: BootbootHeader;
  static fb: ffi::c_void;
}
