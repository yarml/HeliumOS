use core::fmt::Display;

pub enum MemoryUnit {
  Byte,
  KibiByte,
  MebiByte,
  GibiByte,
  TebiByte,
  PebiByte,
  ExbiByte,
}

#[repr(transparent)]
pub struct MemorySize {
  inner: usize,
}

impl MemorySize {
  pub fn new(size: usize) -> Self {
    Self { inner: size }
  }
}

impl Display for MemorySize {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    if self.inner == 0 {
      write!(f, "0{}", MemoryUnit::Byte.suffix())
    } else {
      for unit in (0..MemoryUnit::MEMORY_ORDERS)
        .rev()
        .filter_map(|order| MemoryUnit::from_order(order))
        .filter(|unit| unit.component(self.inner) != 0)
      {
        match write!(f, "{}{}", unit.component(self.inner), unit.suffix()) {
          Err(e) => return Err(e),
          _ => {}
        }
      }
      Ok(())
    }
  }
}

impl MemoryUnit {
  const MEMORY_ORDERS: usize = 7;
  const MEMORY_UNITS: [char; Self::MEMORY_ORDERS] =
    ['B', 'K', 'M', 'G', 'T', 'P', 'E'];
  const MEMORY_SIZES: [usize; Self::MEMORY_ORDERS] = [
    1,
    1024,
    1024 * 1024,
    1024 * 1024 * 1024,
    1024 * 1024 * 1024 * 1024,
    1024 * 1024 * 1024 * 1024 * 1024,
    1024 * 1024 * 1024 * 1024 * 1024 * 1024,
  ];
}

impl MemoryUnit {
  pub const fn from_order(order: usize) -> Option<Self> {
    match order {
      0 => Some(Self::Byte),
      1 => Some(Self::KibiByte),
      2 => Some(Self::MebiByte),
      3 => Some(Self::GibiByte),
      4 => Some(Self::TebiByte),
      5 => Some(Self::PebiByte),
      6 => Some(Self::ExbiByte),
      _ => None,
    }
  }
}

impl MemoryUnit {
  pub const fn order(&self) -> usize {
    match self {
      MemoryUnit::Byte => 0,
      MemoryUnit::KibiByte => 1,
      MemoryUnit::MebiByte => 2,
      MemoryUnit::GibiByte => 3,
      MemoryUnit::TebiByte => 4,
      MemoryUnit::PebiByte => 5,
      MemoryUnit::ExbiByte => 6,
    }
  }
  pub const fn suffix(&self) -> char {
    Self::MEMORY_UNITS[self.order()]
  }
  pub const fn size(&self) -> usize {
    Self::MEMORY_SIZES[self.order()]
  }
  pub const fn component(&self, size: usize) -> usize {
    (size / self.size()) % 1024
  }
}
