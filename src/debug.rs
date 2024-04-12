use core::fmt::{self, Write};

use spin::RwLock;
use x86_64::instructions::port::PortWriteOnly;
pub struct DebugWriter();

#[macro_export]
macro_rules! print {
  ($($arg:tt)*) => ({
    use core::fmt::Write;
    let mut writer = $crate::debug::DEBUG_WRITER.write();
    writer.write_fmt(format_args!($($arg)*)).unwrap();
  });
}

#[macro_export]
macro_rules! println {
  ($fmt:expr) => ($crate::print!(concat!($fmt, "\r\n")));
  ($fmt:expr, $($arg:tt)*) => ($crate::print!(concat!($fmt, "\n"), $($arg)*));
}

pub static DEBUG_WRITER: RwLock<DebugWriter> = RwLock::new(DebugWriter());

impl DebugWriter {
  const DEBUG_PORT: u16 = 0xE9;
  pub fn write_byte(&self, b: u8) {
    let mut debug_port = PortWriteOnly::<u8>::new(Self::DEBUG_PORT);
    unsafe { debug_port.write(b) };
  }
}

impl Write for DebugWriter {
  fn write_str(&mut self, s: &str) -> fmt::Result {
    for b in s.bytes() {
      self.write_byte(b);
    }
    Ok(())
  }
}
