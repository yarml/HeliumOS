pub mod size;

use super::PhysAddr;
use core::{
  fmt::{Debug, Display},
  marker::PhantomData,
};
use size::FrameSize;

#[derive(Clone, Copy)]
#[repr(transparent)]
pub struct Frame<S: FrameSize> {
  boundary: PhysAddr,
  _phantom: PhantomData<S>,
}

impl<S: FrameSize> Frame<S> {
  #[inline]
  pub const fn containing(addr: &PhysAddr) -> Self {
    Self {
      boundary: PhysAddr::new_truncate(addr.as_usize() << S::SHIFT >> S::SHIFT),
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
    self.boundary.as_usize() << S::SHIFT
  }
}

impl<S: FrameSize> Debug for Frame<S> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(f, "Frame{}#{}@{}", S::SIZE, self.number(), self.boundary())
  }
}

impl<S: FrameSize> Display for Frame<S> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(f, "Frame{}#{}", S::SIZE, self.number())
  }
}
