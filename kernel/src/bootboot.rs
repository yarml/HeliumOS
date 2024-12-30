pub mod mmap;

mod framebuffer;
mod header;

pub use header::Header as BootbootHeader;

use core::ffi;

extern "C" {
  static bootboot: BootbootHeader;
  static fb: ffi::c_void;
}
