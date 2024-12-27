use super::{
  serialize::{IOReadable, IOWritable},
  RawPort,
};

pub trait PortNumber {
  fn port<'a>(&'a mut self) -> &'a mut RawPort;
}

pub trait PortReadAccess<T: IOReadable>: PortNumber {
  fn read(&mut self) -> T {
    self.port().read()
  }
}

pub trait PortWriteAccess<T: IOWritable>: PortNumber {
  fn write(&mut self, val: &T) {
    self.port().write(val)
  }
}
