pub mod frame;

mod addr;
mod lowmem;
mod middlemem;

pub use addr::PhysAddr;
use {
  frame::{
    size::{Frame4KiB, Frame64KiB},
    Frame,
  },
  lowmem::LOWMEM_ALLOCATOR,
  middlemem::MIDDLEMEM_ALLOCATOR,
};

use crate::{
  bootboot::{mmap::MmapClass, BootbootHeader},
  debug,
  mem::MemorySize,
};

pub fn init() {
  let mmap = BootbootHeader::instance().mmap();

  let mut totals = [0; 3];
  let mut bounds: [(Option<PhysAddr>, Option<PhysAddr>); 3] = [(None, None); 3];

  let mut lowmem_allocator = LOWMEM_ALLOCATOR.lock();

  for entry in mmap.iter() {
    if entry.is_free() {
      match entry.class() {
        MmapClass::Low => {
          let k64count = entry.size() / (64 * 1024);
          let start: Frame<Frame64KiB> = Frame::containing(&entry.phys_addr());
          for i in 0..k64count {
            lowmem_allocator.free64(start + i);
          }
        }
        MmapClass::Middle => {
          let frame_count = entry.size() / 4096;
          let start: Frame<Frame4KiB> = Frame::containing(&entry.phys_addr());
          MIDDLEMEM_ALLOCATOR.free4_many(start, frame_count);
        }
        MmapClass::High => {}
        _ => unreachable!(),
      }
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

  let pretend_alloc0 = MIDDLEMEM_ALLOCATOR.alloc4().unwrap();
  let pretend_alloc1 = MIDDLEMEM_ALLOCATOR.alloc4().unwrap();

  debug!("Allocated: {} & {}", pretend_alloc0, pretend_alloc1);

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
