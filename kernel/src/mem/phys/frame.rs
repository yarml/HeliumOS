use core::fmt::{Debug, Display};

use super::PhysAddr;

#[derive(Clone, Copy)]
#[repr(transparent)]
pub struct Frame {
  boundary: PhysAddr,
}

impl Frame {
  #[inline]
  pub const fn containing(addr: &PhysAddr) -> Self {
    Self {
      boundary: PhysAddr::new_truncate(addr.as_usize() << 3 >> 3),
    }
  }
}

impl Frame {
  #[inline]
  pub const fn boundary(&self) -> PhysAddr {
    self.boundary
  }

  #[inline]
  pub const fn number(&self) -> usize {
    self.boundary.as_usize() << 3
  }
}

impl Debug for Frame {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(f, "Frame#{}@{}", self.number(), self.boundary())
  }
}

impl Display for Frame {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(f, "Frame#{}", self.number())
  }
}
