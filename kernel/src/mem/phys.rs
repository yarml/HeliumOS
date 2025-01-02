pub mod frame;

mod addr;
mod lowmem;
mod middlemem;

pub use addr::PhysAddr;

use crate::{
  bootboot::{mmap::MmapClass, BootbootHeader},
  debug,
  mem::MemorySize,
};

pub fn init() {
  let mmap = BootbootHeader::instance().mmap();

  let mut totals = [0; 3];
  let mut bounds: [(Option<PhysAddr>, Option<PhysAddr>); 3] = [(None, None); 3];

  for entry in mmap.iter() {
    if entry.is_free() {
      let index = match entry.class() {
        MmapClass::Low => 0,
        MmapClass::Middle => 1,
        MmapClass::High => 2,
        _ => unreachable!(),
      };
      totals[index] += entry.size();
      let start = entry.phys_addr().as_usize();
      let end = entry.phys_addr_adjacent().as_usize();
      if bounds[index].0.is_none_or(|v| v.as_usize() > start) {
        bounds[index].0 = Some(PhysAddr::new_truncate(start))
      }
      if bounds[index].1.is_none_or(|v| v.as_usize() < end) {
        bounds[index].1 = Some(PhysAddr::new_truncate(end))
      }
    }
  }

  let pretend_size = |index: usize| {
    bounds[index].1.unwrap().as_usize() - bounds[index].0.unwrap().as_usize()
  };

  let holes = |index: usize| pretend_size(index) - totals[index];

  let bitmap_overhead = |index: usize| pretend_size(index) / 4096 / 8;
  let buddy_overhead = |index: usize| bitmap_overhead(index) * 2;

  let report = |title: &str, index: usize| {
    debug!("{title}");
    debug!(
      "    Bounds: {}-{}",
      bounds[index].0.unwrap(),
      bounds[index].1.unwrap()
    );
    debug!("    Potential: {}", MemorySize::new(pretend_size(index)));
    debug!("    Usable: {}", MemorySize::new(totals[index]));
    debug!("    Holes: {}", MemorySize::new(holes(index)));
    debug!(
      "    Bitmap Overhead: {}",
      MemorySize::new(bitmap_overhead(index))
    );
    debug!(
      "    Buddy Overhead: {}",
      MemorySize::new(buddy_overhead(index))
    );
  };

  report("Low Memory", 0);
  report("Middle Memory", 1);
  report("High Memory", 2);
}
