use super::RawPort;

pub trait IOReadable {
  fn io_read(port: &mut RawPort) -> Self;
}

pub trait IOWritable {
  fn io_write(&self, port: &mut RawPort);
}

impl IOReadable for u8 {
  fn io_read(port: &mut RawPort) -> Self {
    port.read_u8()
  }
}

impl IOReadable for u16 {
  fn io_read(port: &mut RawPort) -> Self {
    port.read_u16()
  }
}

impl IOReadable for u32 {
  fn io_read(port: &mut RawPort) -> Self {
    port.read_u32()
  }
}

impl IOReadable for i8 {
  fn io_read(port: &mut RawPort) -> Self {
    port.read_i8()
  }
}

impl IOReadable for i16 {
  fn io_read(port: &mut RawPort) -> Self {
    port.read_i16()
  }
}

impl IOReadable for i32 {
  fn io_read(port: &mut RawPort) -> Self {
    port.read_i32()
  }
}

impl IOWritable for u8 {
  fn io_write(&self, port: &mut RawPort) {
    port.write_u8(*self)
  }
}

impl IOWritable for u16 {
  fn io_write(&self, port: &mut RawPort) {
    port.write_u16(*self)
  }
}

impl IOWritable for u32 {
  fn io_write(&self, port: &mut RawPort) {
    port.write_u32(*self)
  }
}

impl IOWritable for i8 {
  fn io_write(&self, port: &mut RawPort) {
    port.write_i8(*self)
  }
}

impl IOWritable for i16 {
  fn io_write(&self, port: &mut RawPort) {
    port.write_i16(*self)
  }
}

impl IOWritable for i32 {
  fn io_write(&self, port: &mut RawPort) {
    port.write_i32(*self)
  }
}
