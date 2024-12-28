use core::fmt::Debug;

use crate::mem::{MemorySize, PhysAddr};

#[repr(C, packed)]
pub struct Entry {
  ptr: u64,
  flags: u64,
}

#[derive(Debug)]
pub enum Type {
  Used,
  Free,
  Acpi,
  Mmio,
}

impl Entry {
  #[inline]
  pub const fn typ(&self) -> Type {
    match self.flags & 0xF {
      0 => Type::Used,
      1 => Type::Free,
      2 => Type::Acpi,
      3 => Type::Mmio,
      _ => unreachable!(),
    }
  }

  #[inline]
  pub const fn phys_addr(&self) -> PhysAddr {
    PhysAddr::new_truncate(self.ptr as usize)
  }
  #[inline]
  pub const fn size(&self) -> usize {
    (self.flags & 0xFFFFFFFFFFFFFFF0) as usize
  }
}

impl Debug for Entry {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(
      f,
      "{:?}({}-{}/{})",
      self.typ(),
      self.phys_addr(),
      self.phys_addr().with_offset(self.size()),
      MemorySize::new(self.size())
    )
  }
}
