mod header;
mod mmap;

extern "C" {
  static bootboot: BootbootHeader;
}

pub use header::Header as BootbootHeader;
pub use mmap::{Entry as MmapEntry, Type as MmapType};
