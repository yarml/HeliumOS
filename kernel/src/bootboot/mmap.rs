mod raw;

pub use raw::RawEntry as MmapRawEntry;
pub use raw::Type as MmapType;

use crate::mem::MemorySize;
use crate::mem::PhysAddr;
use core::fmt::Debug;
use core::usize;

#[derive(Clone, Copy)]
pub struct MmapEntry {
  start: PhysAddr,
  size: usize,
  typ: MmapType,
  class: MmapClass,
  gap: Option<usize>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MmapClass {
  Unusable,
  Low,
  Middle,
  High,
}

pub struct PhysMemoryMap {
  mmap: &'static [MmapRawEntry],
}

pub struct PhysMemoryMapIterator {
  mmap: &'static [MmapRawEntry],
  current: usize,
  offset: usize,
}

impl PhysMemoryMap {
  #[inline]
  pub const fn from(mmap: &'static [MmapRawEntry]) -> Self {
    Self { mmap }
  }
}

impl PhysMemoryMap {
  #[inline]
  pub const fn iter(&self) -> PhysMemoryMapIterator {
    PhysMemoryMapIterator {
      mmap: self.mmap,
      current: 0,
      offset: 0,
    }
  }
}

impl Iterator for PhysMemoryMapIterator {
  type Item = MmapEntry;

  fn next(&mut self) -> Option<Self::Item> {
    if self.current >= self.mmap.len() {
      return None;
    }
    let current = &self.mmap[self.current];
    let begin_class =
      MmapClass::from_address(current.phys_addr().with_offset(self.offset));
    let end_class = MmapClass::from_address(current.phys_addr_end());
    if begin_class == end_class {
      let gap = if self.current + 1 < self.mmap.len() {
        let next = &self.mmap[self.current + 1];
        Some(
          next.phys_addr().as_usize() - current.phys_addr_adjacent().as_usize(),
        )
      } else {
        None
      };
      let current = MmapEntry::from_raw(current, self.offset, gap, usize::MAX);
      self.current += 1;
      self.offset = 0;
      Some(current)
    } else {
      let size = begin_class.adjacent().as_usize()
        - current.phys_addr().as_usize()
        - self.offset;
      let current = MmapEntry::from_raw(current, self.offset, None, size);
      self.offset += size;
      Some(current)
    }
  }
}

impl MmapEntry {
  pub fn from_raw(
    raw: &MmapRawEntry,
    offset: usize,
    gap: Option<usize>,
    max_size: usize,
  ) -> Self {
    let start = raw.phys_addr().with_offset(offset);
    let typ = raw.typ();
    Self {
      start,
      size: usize::min(raw.size() - offset, max_size),
      typ,
      class: MmapClass::from_address(start).oftype(typ),
      gap,
    }
  }
}

impl MmapEntry {
  #[inline]
  pub const fn typ(&self) -> MmapType {
    self.typ
  }
  #[inline]
  pub const fn class(&self) -> MmapClass {
    self.class
  }
  /// Inclusive first address within this entry
  #[inline]
  pub const fn phys_addr(&self) -> PhysAddr {
    self.start
  }
  /// Exclusive last address within this entry
  #[inline]
  pub const fn phys_addr_adjacent(&self) -> PhysAddr {
    self.start.with_offset(self.size)
  }
  /// Inclusive last address within this entry
  #[inline]
  pub const fn phys_addr_end(&self) -> PhysAddr {
    self.start.with_offset(self.size - 1)
  }
  #[inline]
  pub const fn size(&self) -> usize {
    self.size
  }

  #[inline]
  pub fn is_free(&self) -> bool {
    self.typ == MmapType::Free && self.class != MmapClass::Unusable
  }
}

impl MmapClass {
  const MIB1: usize = 1024 * 1024;
  const MIB16: usize = 16 * 1024 * 1024;
  const GIB4: usize = 4 * 1024 * 1024 * 1024;

  #[inline]
  pub const fn from_address(addr: PhysAddr) -> Self {
    match addr.as_usize() {
      0..Self::MIB1 => Self::Unusable,
      Self::MIB1..Self::MIB16 => Self::Low,
      Self::MIB16..Self::GIB4 => Self::Middle,
      _ => Self::High,
    }
  }

  #[inline]
  pub const fn oftype(self, typ: MmapType) -> Self {
    match typ {
      MmapType::Free => self,
      _ => MmapClass::Unusable,
    }
  }
}

impl MmapClass {
  #[inline]
  pub const fn adjacent(&self) -> PhysAddr {
    PhysAddr::new_truncate(match self {
      MmapClass::Unusable => Self::MIB1,
      MmapClass::Low => Self::MIB16,
      MmapClass::Middle => Self::GIB4,
      MmapClass::High => 0,
    })
  }
}

impl Debug for MmapEntry {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(
      f,
      "{:?}@{:?}({}-{}/{})",
      self.typ(),
      self.class(),
      self.phys_addr(),
      self.phys_addr_adjacent(),
      MemorySize::new(self.size())
    )?;
    if let Some(gap) = self.gap {
      write!(f, ", Gap: {}", MemorySize::new(gap))
    } else {
      Ok(())
    }
  }
}
