use core::{
  fmt::{Debug, Display},
  usize,
};

use super::frame::{size::FrameSize, Frame};

macro_rules! phys_truncated {
  ($addr:expr) => {
    $addr & 0x00FFFFFFFFFFFFFF
  };
}

#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct PhysAddr {
  inner: usize,
}

impl PhysAddr {
  pub const MAX: Self = Self::new_truncate(usize::MAX);
  pub const MIN: Self = Self::zero();

  #[inline]
  pub const fn new(addr: usize) -> Option<Self> {
    if addr == phys_truncated!(addr) {
      Some(Self { inner: addr })
    } else {
      None
    }
  }
  #[inline]
  pub const fn new_truncate(addr: usize) -> Self {
    Self {
      inner: phys_truncated!(addr),
    }
  }
  #[inline]
  pub const fn zero() -> Self {
    Self { inner: 0 }
  }
}

impl PhysAddr {
  #[inline]
  pub const fn add(&self, offset: usize) -> Option<Self> {
    Self::new(self.inner + offset)
  }
  #[inline]
  pub const fn add_truncate(&self, offset: usize) -> Self {
    Self::new_truncate(self.inner + offset)
  }

  #[inline]
  pub const fn frame<S: FrameSize>(&self) -> Frame<S> {
    Frame::containing(self)
  }
}

impl PhysAddr {
  #[inline]
  pub const fn as_usize(&self) -> usize {
    self.inner
  }
  #[inline]
  pub const fn as_u64(&self) -> u64 {
    self.inner as u64
  }
}

impl From<usize> for PhysAddr {
  #[inline]
  fn from(value: usize) -> Self {
    Self::new_truncate(value)
  }
}

impl From<u64> for PhysAddr {
  #[inline]
  fn from(value: u64) -> Self {
    Self::new_truncate(value as usize)
  }
}

impl From<PhysAddr> for usize {
  #[inline]
  fn from(value: PhysAddr) -> Self {
    value.as_usize()
  }
}

impl From<PhysAddr> for u64 {
  #[inline]
  fn from(value: PhysAddr) -> Self {
    value.as_u64()
  }
}

impl Debug for PhysAddr {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(f, "PhysAddr({:016x})", self.inner)
  }
}

impl Display for PhysAddr {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(f, "{:016x}", self.inner)
  }
}
