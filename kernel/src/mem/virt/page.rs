use core::{
  fmt::{Debug, Display},
  marker::PhantomData,
};

use size::PageSize;

use crate::mem::MemorySize;

use super::VirtAddr;

pub mod size;

#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct Page<S: PageSize> {
  boundary: VirtAddr,
  _phantom: PhantomData<S>,
}

impl<S: PageSize> Page<S> {
  #[inline]
  pub const fn containing(addr: &VirtAddr) -> Self {
    Self {
      boundary: VirtAddr::new_truncate(addr.as_usize() & S::MASK),
      _phantom: PhantomData,
    }
  }
}

impl<S: PageSize> Page<S> {
  #[inline]
  pub const fn boundary(&self) -> VirtAddr {
    self.boundary
  }

  #[inline]
  pub const fn number(&self) -> usize {
    self.boundary.as_usize() >> S::SHIFT
  }
}

impl<S: PageSize> Debug for Page<S> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(
      f,
      "Page{}#{}@{}",
      MemorySize::new(S::SIZE),
      self.number(),
      self.boundary()
    )
  }
}

impl<S: PageSize> Display for Page<S> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(f, "Page{}#{}", MemorySize::new(S::SIZE), self.number())
  }
}
