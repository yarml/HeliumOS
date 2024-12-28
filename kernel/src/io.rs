mod abstraction;
mod raw;
mod serialize;

pub use abstraction::{PortNumber, PortReadAccess, PortWriteAccess};
pub use raw::RawPort;

use core::marker::PhantomData;
use serialize::{IOReadable, IOWritable};

pub struct PortReadOnly<T: IOReadable> {
  port: RawPort,
  phantom: PhantomData<T>,
}

pub struct PortWriteOnly<T: IOWritable> {
  port: RawPort,
  phantom: PhantomData<T>,
}

pub struct PortReadWrite<T: IOReadable + IOWritable> {
  port: RawPort,
  phantom: PhantomData<T>,
}

impl<T: IOReadable> PortReadOnly<T> {
  /// # Safety
  /// Must be the only instance of PortRead, PortWrite, PortReadWrite, or RawPort with the specified port address
  #[inline]
  pub const unsafe fn new(port: u16) -> Self {
    Self {
      port: unsafe {
        // # Safety
        // This is the only instance of RawPort with the speicifed port address
        RawPort::new(port)
      },
      phantom: PhantomData::<T>,
    }
  }
}

impl<T: IOWritable> PortWriteOnly<T> {
  /// # Safety
  /// Must be the only instance of PortRead, PortWrite, PortReadWrite, or RawPort with the specified port address
  #[inline]
  pub const unsafe fn new(port: u16) -> Self {
    Self {
      port: unsafe {
        // # Safety
        // This is the only instance of RawPort with the speicifed port address
        RawPort::new(port)
      },
      phantom: PhantomData::<T>,
    }
  }
}

impl<T: IOReadable + IOWritable> PortReadWrite<T> {
  /// # Safety
  /// Must be the only instance of PortRead, PortWrite, PortReadWrite, or RawPort with the specified port address
  #[inline]
  pub const unsafe fn new(port: u16) -> Self {
    Self {
      port: unsafe {
        // # Safety
        // This is the only instance of RawPort with the speicifed port address
        RawPort::new(port)
      },
      phantom: PhantomData::<T>,
    }
  }
}

impl<T: IOReadable> PortNumber for PortReadOnly<T> {
  fn port<'a>(&'a mut self) -> &'a mut RawPort {
    &mut self.port
  }
}

impl<T: IOWritable> PortNumber for PortWriteOnly<T> {
  fn port<'a>(&'a mut self) -> &'a mut RawPort {
    &mut self.port
  }
}

impl<T: IOReadable + IOWritable> PortNumber for PortReadWrite<T> {
  fn port<'a>(&'a mut self) -> &'a mut RawPort {
    &mut self.port
  }
}

impl<T: IOReadable> PortReadAccess<T> for PortReadOnly<T> {}
impl<T: IOWritable> PortWriteAccess<T> for PortWriteOnly<T> {}
impl<T: IOReadable + IOWritable> PortReadAccess<T> for PortReadWrite<T> {}
impl<T: IOReadable + IOWritable> PortWriteAccess<T> for PortReadWrite<T> {}
