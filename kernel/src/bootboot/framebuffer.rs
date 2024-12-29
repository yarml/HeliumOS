static MASTER_FB_LOCK: AtomicBool = AtomicBool::new(false);

use crate::mem::PhysAddr;
use core::{
  ops::{Deref, DerefMut},
  slice,
  sync::atomic::{AtomicBool, Ordering},
};

#[derive(Clone, Copy)]
pub struct FramebufferInfo {
  pub(super) typ: FramebufferType,
  pub(super) phys_location: PhysAddr,
  pub(super) location: *mut u8,
  pub(super) size: usize,
  pub(super) width: usize,
  pub(super) height: usize,
  pub(super) scanline: usize,
}

pub struct FramebufferAccess<'a> {
  buffer: &'a mut [u8],
}

#[derive(Clone, Copy)]
pub enum FramebufferType {
  Argb,
  Rgba, // Knee
  Abgr,
  Bgra, // Cow
}

impl FramebufferInfo {
  pub fn typ(&self) -> FramebufferType {
    self.typ
  }
  pub fn phys(&self) -> PhysAddr {
    self.phys_location
  }
  pub fn size(&self) -> usize {
    self.size
  }
  pub fn width(&self) -> usize {
    self.width
  }
  pub fn height(&self) -> usize {
    self.height
  }
  pub fn scanline(&self) -> usize {
    self.scanline
  }
  pub fn acquire_buffer<'a>(&'a self) -> FramebufferAccess<'a> {
    while MASTER_FB_LOCK
      .compare_exchange(false, true, Ordering::Acquire, Ordering::Relaxed)
      .is_err()
    {}
    FramebufferAccess {
      buffer: unsafe {
        // # Safety
        // Uniqueness is guarenteed by MASTER_FB_LOCK
        slice::from_raw_parts_mut(self.location, self.size)
      },
    }
  }
}

impl TryFrom<u8> for FramebufferType {
  type Error = u8;

  fn try_from(value: u8) -> Result<Self, Self::Error> {
    match value {
      0 => Ok(FramebufferType::Argb),
      1 => Ok(FramebufferType::Rgba),
      2 => Ok(FramebufferType::Abgr),
      3 => Ok(FramebufferType::Bgra),
      v => Err(v),
    }
  }
}

impl<'a> Drop for FramebufferAccess<'a> {
  fn drop(&mut self) {
    MASTER_FB_LOCK.store(false, Ordering::Release);
  }
}

impl<'a> Deref for FramebufferAccess<'a> {
  type Target = [u8];

  fn deref(&self) -> &Self::Target {
    self.buffer
  }
}

impl<'a> DerefMut for FramebufferAccess<'a> {
  fn deref_mut(&mut self) -> &mut Self::Target {
    self.buffer
  }
}
