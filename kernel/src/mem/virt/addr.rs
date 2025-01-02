use core::{
  fmt::{Debug, Display},
  usize,
};

macro_rules! make_canonical {
  ($addr:expr) => {
    if $addr & 0x0001000000000000 == 0 {
      $addr & 0x0000FFFFFFFFFFFF
    } else {
      $addr | 0xFFFF000000000000
    }
  };
}

#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct VirtAddr {
  inner: usize,
}

impl VirtAddr {
  pub const MAX: Self = Self::new_truncate(usize::MAX);
  pub const MIN: Self = Self::zero();
  #[inline]
  pub const fn new(addr: usize) -> Option<Self> {
    if addr == make_canonical!(addr) {
      Some(Self { inner: addr })
    } else {
      None
    }
  }
  #[inline]
  pub const fn new_truncate(addr: usize) -> Self {
    Self {
      inner: make_canonical!(addr),
    }
  }
  #[inline]
  pub const fn zero() -> Self {
    Self { inner: 0 }
  }
}

impl VirtAddr {
  #[inline]
  pub const fn add(&self, offset: usize) -> Option<Self> {
    Self::new(self.inner + offset)
  }
  #[inline]
  pub const fn add_truncate(&self, offset: usize) -> Self {
    Self::new_truncate(self.inner + offset)
  }
}

impl VirtAddr {
  #[inline]
  pub const fn as_usize(&self) -> usize {
    self.inner
  }
  #[inline]
  pub const fn as_u64(&self) -> u64 {
    self.inner as u64
  }
  #[inline]
  pub const fn as_ptr<T>(&self) -> *const T {
    self.inner as *const T
  }
  #[inline]
  pub const fn as_ptr_mut<T>(&self) -> *mut T {
    self.inner as *mut T
  }
  /// # Safety
  /// Must ensure same safety guarentees as (*const T)::as_ref
  /// Additionally, must also ensure that the virtual address is currently
  /// mapped.
  pub const unsafe fn as_ref<'a, 'b, T>(&'a self) -> Option<&'b T> {
    unsafe {
      // # Safety
      // Guarenteed by caller
      self.as_ptr::<T>().as_ref()
    }
  }
  /// # Safety
  /// Must ensure same safety guarentees as (*mut T)::as_mut
  /// Additionally, must also ensure that the virtual address is currently
  /// mapped.
  pub const unsafe fn as_mut<'a, 'b, T>(&'a self) -> Option<&'b mut T> {
    unsafe {
      // # Safety
      // Guarenteed by caller
      self.as_ptr_mut::<T>().as_mut()
    }
  }
}

impl From<usize> for VirtAddr {
  #[inline]
  fn from(value: usize) -> Self {
    Self::new_truncate(value)
  }
}

impl From<u64> for VirtAddr {
  #[inline]
  fn from(value: u64) -> Self {
    Self::new_truncate(value as usize)
  }
}

impl<T> From<*const T> for VirtAddr {
  fn from(value: *const T) -> Self {
    Self {
      inner: value as usize,
    }
  }
}

impl<T> From<*mut T> for VirtAddr {
  fn from(value: *mut T) -> Self {
    Self {
      inner: value as usize,
    }
  }
}

impl From<VirtAddr> for usize {
  #[inline]
  fn from(value: VirtAddr) -> Self {
    value.as_usize()
  }
}

impl From<VirtAddr> for u64 {
  #[inline]
  fn from(value: VirtAddr) -> Self {
    value.as_u64()
  }
}

impl<T> From<VirtAddr> for *const T {
  fn from(value: VirtAddr) -> Self {
    value.as_ptr()
  }
}

impl<T> From<VirtAddr> for *mut T {
  fn from(value: VirtAddr) -> Self {
    value.as_ptr_mut()
  }
}

impl Debug for VirtAddr {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(f, "VirtAddr({:016x})", self.inner)
  }
}

impl Display for VirtAddr {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    write!(f, "{:016x}", self.inner)
  }
}
