use crate::{
  bootboot::{bootboot, fb_virt},
  println,
};
use core::mem;
use spin::{Mutex, Once};

static FRAMEBUFFER: Once<Mutex<Framebuffer>> = Once::new();

pub struct Framebuffer {
  // format: FramebufferFormat,
  // width: usize,
  // height: usize,
  // scanline: usize,
  // ptr: VirtAddr,
  // // Double buffer info
  // // Lowest and highest offsets written to after last refresh
  // lowest_offset: Option<usize>,
  // highest_offset: Option<usize>,
  // double_buffer: Vec<u8>,
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
    // let bootboot = bootboot();
    // let width = bootboot.fb_width as usize;
    // let height = bootboot.fb_height as usize;
    // let scanline = bootboot.fb_scanline as usize;
    // let double_buffer = vec![0u8; scanline * height];

    let fb_virt = fb_virt();
    println!("Framebuffer virtual address: {:?}", fb_virt);

    Self {
      // format: bootboot.fb_type.try_into().unwrap(),
      // width,
      // height,
      // scanline,
      // ptr: fb_virt,
      // lowest_offset: None,
      // highest_offset: None,
      // double_buffer,
    }
  }

  // pub fn instance<'a>() -> MutexGuard<'a, Framebuffer> {
  //   FRAMEBUFFER.get().unwrap().lock()
  // }

  // pub fn set_pixel(&mut self, x: usize, y: usize, pixel: PixelColor) {
  //   let offset = y * self.scanline + x * mem::size_of::<u32>();
  //   let word = pixel.word(self.format);
  //   assert!(x < self.width);
  //   assert!(y < self.height);
  //   debug_assert!(offset < self.double_buffer.len());
  //   println!("Setting pixel at offset {}", offset);

  //   if offset < *self.lowest_offset.get_or_insert(offset) {
  //     self.lowest_offset = Some(offset);
  //   }
  //   if offset > *self.highest_offset.get_or_insert(offset + 4) {
  //     self.highest_offset = Some(offset + 4);
  //   }

  //   unsafe {
  //     (self.double_buffer.as_mut_ptr() as *mut u32)
  //       .add(offset)
  //       .write_volatile(word)
  //   };
  // }

  // pub fn refresh(&mut self) {
  //   let lowest_offset = self.lowest_offset.take().unwrap_or(0);
  //   let highest_offset = self.highest_offset.take().unwrap_or(0);
  //   let copy_len = highest_offset - lowest_offset;
  //   unsafe {
  //     self
  //       .ptr
  //       .as_mut_ptr::<u8>()
  //       .add(lowest_offset)
  //       .copy_from_nonoverlapping(
  //         self.double_buffer.as_ptr().add(lowest_offset),
  //         copy_len,
  //       )
  //   };
  // }
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

impl From<(u8, u8, u8, u8)> for PixelColor {
  fn from((red, green, blue, alpha): (u8, u8, u8, u8)) -> Self {
    PixelColor {
      red,
      green,
      blue,
      alpha,
    }
  }
}

impl From<(u8, u8, u8)> for PixelColor {
  fn from((red, green, blue): (u8, u8, u8)) -> Self {
    PixelColor {
      red,
      green,
      blue,
      alpha: 255,
    }
  }
}

pub(super) fn init() {
  FRAMEBUFFER.call_once(|| Mutex::new(Framebuffer::from_bootboot()));
  debug_set_pixel(0, 0, (255, 255, 255).into());
}

pub fn debug_set_pixel(x: usize, y: usize, pixel: PixelColor) {
  let bootboot = bootboot();
  let ptr = fb_virt();
  let offset = y * bootboot.fb_scanline as usize + x * mem::size_of::<u32>();
  let word = pixel.word(FramebufferFormat::Argb);
  unsafe {
    ptr.as_mut_ptr::<u32>().add(offset).write_volatile(word);
  };
}
