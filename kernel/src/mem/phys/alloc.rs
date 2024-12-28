use super::PhysAddr;

static ALLOCATOR: usize = 0;

pub struct FrameAllocator {}

pub struct FrameAllocation {
  start: PhysAddr,
  size: usize,
}
