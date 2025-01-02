use crate::mem::PhysAddr;
use core::fmt::Debug;

#[repr(C, packed)]
pub struct RawEntry {
  ptr: u64,
  flags: u64,
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum Type {
  Used,
  Free,
  Acpi,
  Mmio,
}

impl RawEntry {
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

  /// Inclusive first address within this entry
  #[inline]
  pub const fn phys_addr(&self) -> PhysAddr {
    PhysAddr::new_truncate(self.ptr as usize)
  }

  /// Inclusive last address within this entry
  #[inline]
  pub const fn phys_addr_end(&self) -> PhysAddr {
    self.phys_addr().add_truncate(self.size() - 1)
  }

  /// Exclusive last address within this entry
  #[inline]
  pub const fn phys_addr_adjacent(&self) -> PhysAddr {
    self.phys_addr().add_truncate(self.size())
  }

  #[inline]
  pub const fn size(&self) -> usize {
    (self.flags & 0xFFFFFFFFFFFFFFF0) as usize
  }

  #[inline]
  pub fn is_free(&self) -> bool {
    self.typ() == Type::Free
  }
}
