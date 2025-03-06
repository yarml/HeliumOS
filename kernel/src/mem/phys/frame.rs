pub mod size;

use core::ops::Add;
use crate::mem::MemorySize;

use {
  super::PhysAddr,
  core::{
    fmt::{Debug, Display},
    marker::PhantomData,
  },
  size::FrameSize,
};

#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct Frame<S: FrameSize> {
  boundary: PhysAddr,
  _phantom: PhantomData<S>,
}

impl<S: FrameSize> Frame<S> {
  #[inline]
  pub const fn containing(addr: &PhysAddr) -> Self {
    Self {
      boundary: PhysAddr::new_truncate(addr.as_usize() & S::MASK),
      _phantom: PhantomData,
    }
  }
  #[inline]
  pub const fn from_number(num: usize) -> Self {
    Self {
      boundary: PhysAddr::new_truncate(num << S::SHIFT),
      _phantom: PhantomData,
    }
  }
}

impl<S: FrameSize> Frame<S> {
  #[inline]
  pub const fn boundary(&self) -> PhysAddr {
    self.boundary
  }

  #[inline]
  pub const fn number(&self) -> usize {
    self.boundary.as_usize() >> S::SHIFT
  }
}

impl<S: FrameSize> Add<usize> for Frame<S> {
  type Output = Frame<S>;

  fn add(self, rhs: usize) -> Self::Output {
    Self::from_number(self.number() + rhs)
  }
}

impl<S: FrameSize> Debug for Frame<S> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(
      f,
      "Frame{}#{}@{}",
      MemorySize::new(S::SIZE),
      self.number(),
      self.boundary()
    )
  }
}

impl<S: FrameSize> Display for Frame<S> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(f, "Frame{}#{}", MemorySize::new(S::SIZE), self.number())
  }
}
