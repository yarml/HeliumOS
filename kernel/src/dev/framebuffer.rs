use core::mem;

use alloc::vec::Vec;
use spin::{Once, RwLock};

use crate::bootboot::bootboot;

pub struct Framebuffer {
  format: FramebufferFormat,
  width: usize,
  height: usize,
  scanline: usize,
  ptr: u64,

  // Double buffer info
  // Lowest and highest offsets written to after last refresh
  lowest_offset: Option<usize>,
  highest_offset: Option<usize>,
  double_buffer: Vec<u8>,
}

pub struct PixelColor {
  red: u8,
  green: u8,
  blue: u8,
  alpha: u8,
}

#[derive(Clone, Copy)]
pub enum FramebufferFormat {
  Argb,
  Idk1,
  Idk2,
  Bgra,
}

impl Framebuffer {
  pub fn from_bootboot() -> Self {
    let bootboot = bootboot();
    let width = bootboot.fb_width as usize;
    let height = bootboot.fb_height as usize;
    let scanline = bootboot.fb_scanline as usize;
    let mut double_buffer = Vec::with_capacity(scanline * height);

    // Fill all with 0s
    // Yes, slow, too lazy to find another way
    for _ in 0..double_buffer.capacity() {
      double_buffer.push_within_capacity(0).unwrap();
    }

    Self {
      format: bootboot.fb_type.try_into().unwrap(),
      width,
      height,
      scanline,
      ptr: bootboot.fb_ptr as u64,
      lowest_offset: None,
      highest_offset: None,
      double_buffer,
    }
  }

  pub fn set_pixel(&mut self, x: usize, y: usize, pixel: PixelColor) {
    let offset = y * self.scanline + x * mem::size_of::<u32>();
    let word = pixel.word(self.format);
    assert!(x < self.width);
    assert!(y < self.height);
    debug_assert!(offset < self.double_buffer.len());

    if offset < *self.lowest_offset.get_or_insert(offset) {
      self.lowest_offset = Some(offset);
    }
    if offset > *self.highest_offset.get_or_insert(offset) {
      self.highest_offset = Some(offset);
    }

    unsafe {
      (self.double_buffer.as_mut_ptr() as *mut u32)
        .offset(offset as isize)
        .write_volatile(word)
    };
  }

  pub fn refresh(&mut self) {
    let len = self.height * self.scanline;
    let lowest_offset = self.lowest_offset.take().or(Some(0)).unwrap();
    let highest_offset = self
      .highest_offset
      .take()
      .or(Some(len))
      .unwrap();
    unsafe {
      (self.ptr as *mut u8).copy_from_nonoverlapping(
        self.double_buffer.as_ptr(),
        self.double_buffer.len(),
      )
    };
  }
}

impl TryInto<FramebufferFormat> for u8 {
  type Error = ();

  fn try_into(self) -> Result<FramebufferFormat, Self::Error> {
    match self {
      0 => Ok(FramebufferFormat::Argb),
      1 => Ok(FramebufferFormat::Idk1),
      2 => Ok(FramebufferFormat::Idk2),
      3 => Ok(FramebufferFormat::Bgra),
      _ => Err(()),
    }
  }
}

impl PixelColor {
  pub fn word(&self, format: FramebufferFormat) -> u32 {
    match format {
      FramebufferFormat::Argb => {
        (self.alpha as u32 & 0xff) << 24
          | (self.red as u32 & 0xff) << 16
          | (self.green as u32 & 0xff) << 8
          | (self.blue as u32 & 0xff)
      }
      FramebufferFormat::Idk1 => todo!(),
      FramebufferFormat::Idk2 => todo!(),
      FramebufferFormat::Bgra => {
        (self.blue as u32 & 0xff) << 24
          | (self.green as u32 & 0xff) << 16
          | (self.red as u32 & 0xff) << 8
          | (self.alpha as u32 & 0xff)
      }
    }
  }
}

impl Into<PixelColor> for (u8, u8, u8, u8) {
  fn into(self) -> PixelColor {
    PixelColor {
      red: self.0,
      green: self.1,
      blue: self.2,
      alpha: self.3,
    }
  }
}

impl Into<PixelColor> for (u8, u8, u8) {
  fn into(self) -> PixelColor {
    PixelColor {
      red: self.0,
      green: self.1,
      blue: self.2,
      alpha: 255,
    }
  }
}

static FRAMEBUFFER: Once<RwLock<Framebuffer>> = Once::new();

pub(super) fn init() {
  FRAMEBUFFER.call_once(|| RwLock::new(Framebuffer::from_bootboot()));
}
