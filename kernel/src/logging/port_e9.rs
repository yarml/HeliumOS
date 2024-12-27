use crate::{
  io::{PortWriteAccess, PortWriteOnly},
  sync::Mutex,
};
use core::fmt::Write;

/// This must never be used directly, and is only an implmentation detail
pub static DEBUG_PORT: Mutex<E9Writer> = Mutex::new(E9Writer::new());

pub struct E9Writer {
  port: PortWriteOnly<u8>,
}

impl E9Writer {
  pub const fn new() -> Self {
    Self {
      port: unsafe {
        // # Safety
        // Assuming nowhere else we will make an IO port at 0xE9
        PortWriteOnly::new(0xE9)
      },
    }
  }
}

impl Write for E9Writer {
  fn write_str(&mut self, s: &str) -> core::fmt::Result {
    for b in s.bytes() {
      self.port.write(&b);
    }
    Ok(())
  }
}
