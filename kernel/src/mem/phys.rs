mod addr;
mod alloc;
mod frame;
mod internal;

pub use addr::PhysAddr;
pub use alloc::FrameAllocation;
pub use frame::Frame;

use crate::{bootboot::BootbootHeader, debug, info, mem::MemorySize};

pub fn init() {
  let mmap = BootbootHeader::instance().mmap();
  let mut last_free = PhysAddr::zero();
  let mut total_size = 0;

  for entry in mmap.iter() {
    if entry.is_free() {
      last_free = entry.phys_addr_end();
      total_size += entry.size();
    }
    debug!("{:?}", entry);
  }

  info!("Available Memory: {}", MemorySize::new(total_size));

  let man_overhead = last_free.as_usize() / 4096 / 8 * 2;
  debug!(
    "Minimum Management Overhead: {}",
    MemorySize::new(man_overhead)
  );
  let ideal_size = man_overhead / 2 * 8 * 4096;
  debug!(
    "Ideal Memory size would have been: {}",
    MemorySize::new(ideal_size)
  );

  let loss = ideal_size - total_size;
  debug!(
    "Losing {} of potetial memory for same overhead",
    MemorySize::new(loss)
  );
}
