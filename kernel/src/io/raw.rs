use core::{arch::asm, mem};

use super::serialize::{IOReadable, IOWritable};

pub struct RawPort {
  port: u16,
}

impl RawPort {
  /// # Safety
  /// No 2 instances of RawPort can share the same port number
  pub const unsafe fn new(port: u16) -> Self {
    Self { port }
  }
}

impl RawPort {
  #[inline]
  pub fn read_u8(&mut self) -> u8 {
    let val: u8;
    unsafe {
      // # Safety
      // Safe as long as only one instance of a RawPort with the specified port
      asm! {
        "in al, dx",
        out("al") val,
        in("dx") self.port,
        options(nostack, nomem, preserves_flags)
      };
    }
    val
  }
  #[inline]
  pub fn read_i8(&mut self) -> i8 {
    unsafe {
      // # Safety
      // All bit combinations of i8 are valid state
      mem::transmute(self.read_u8())
    }
  }
  #[inline]
  pub fn read_u16(&mut self) -> u16 {
    let val: u16;
    unsafe {
      // # Safety
      // Safe as long as only one instance of a RawPort with the specified port
      asm! {
        "in ax, dx",
        out("ax") val,
        in("dx") self.port,
        options(nostack, nomem, preserves_flags)
      };
    }
    val
  }
  #[inline]
  pub fn read_i16(&mut self) -> i16 {
    unsafe {
      // # Safety
      // All bit combinations of i16 are valid state
      mem::transmute(self.read_u16())
    }
  }
  #[inline]
  pub fn read_u32(&mut self) -> u32 {
    let val: u32;
    unsafe {
      // # Safety
      // Safe as long as only one instance of a RawPort with the specified port
      asm! {
        "in eax, dx",
        out("eax") val,
        in("dx") self.port,
        options(nostack, nomem, preserves_flags)
      };
    }
    val
  }
  #[inline]
  pub fn read_i32(&mut self) -> i32 {
    unsafe {
      // # Safety
      // All bit combinations of i32 are valid state
      mem::transmute(self.read_u32())
    }
  }
  #[inline]
  pub fn write_u8(&mut self, val: u8) {
    unsafe {
      // # Safety
      // Safe as long as only one instance of a RawPort with the specified port
      asm! {
        "out dx, al",
        in("dx") self.port,
        in("al") val,
        options(nostack, nomem, preserves_flags)
      }
    }
  }
  #[inline]
  pub fn write_i8(&mut self, val: i8) {
    self.write_u8(unsafe {
      // # Safety
      // All bit combinations of u8 are valid state
      mem::transmute(val)
    });
  }
  #[inline]
  pub fn write_u16(&mut self, val: u16) {
    unsafe {
      // # Safety
      // Safe as long as only one instance of a RawPort with the specified port
      asm! {
        "out dx, ax",
        in("dx") self.port,
        in("ax") val,
        options(nostack, nomem, preserves_flags)
      }
    }
  }
  #[inline]
  pub fn write_i16(&mut self, val: i16) {
    self.write_u16(unsafe {
      // # Safety
      // All bit combinations of u16 are valid state
      mem::transmute(val)
    });
  }
  #[inline]
  pub fn write_u32(&mut self, val: u32) {
    unsafe {
      // # Safety
      // Safe as long as only one instance of a RawPort with the specified port
      asm! {
        "out dx, eax",
        in("dx") self.port,
        in("eax") val,
        options(nostack, nomem, preserves_flags)
      }
    }
  }
  #[inline]
  pub fn write_i32(&mut self, val: i32) {
    self.write_u32(unsafe {
      // # Safety
      // All bit combinations of u32 are valid state
      mem::transmute(val)
    });
  }

  #[inline]
  pub fn read<T: IOReadable>(&mut self) -> T {
    T::io_read(self)
  }

  #[inline]
  pub fn write<T: IOWritable>(&mut self, val: &T) {
    val.io_write(self)
  }
}
