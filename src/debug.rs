use core::fmt;

use spin::Mutex;

use crate::io;

pub struct DebugWriter();

#[macro_export]
macro_rules! print {
  ($($arg:tt)*) => ({
    use core::fmt::Write;
    let mut writer = $crate::debug::DEBUG_WRITER.lock();
    writer.write_fmt(format_args!($($arg)*)).unwrap();
});
}

pub static DEBUG_WRITER: Mutex<DebugWriter> = Mutex::new(DebugWriter());

impl DebugWriter {
  const DEBUG_PORT: u16 = 0xE9;
  pub fn write_byte(&self, b: u8) {
    io::outb(Self::DEBUG_PORT, b);
  }
}

impl fmt::Write for DebugWriter {
  fn write_str(&mut self, s: &str) -> fmt::Result {
    for b in s.bytes() {
      self.write_byte(b);
    }
    Ok(())
  }
}