mod fb;
mod port_e9;

#[doc(hidden)]
pub use port_e9::DEBUG_PORT;

#[doc(hidden)]
pub use fb::FRAMEBUFFER;

/// This must never be used directly, and is only an implmentation detail
#[doc(hidden)]
#[macro_export]
macro_rules! log_dbg {
  ($header:expr, $($arg:tt)*) => ({
    use core::fmt::Write;
    use $crate::logging::DEBUG_PORT;
    let mut debug_port = DEBUG_PORT.lock();
    debug_port.write_fmt(format_args!("{}: ", $header)).unwrap();
    debug_port.write_fmt(format_args!($($arg)*)).unwrap();
    debug_port.write_str("\r\n").unwrap();

  });
}

#[macro_export]
macro_rules! log_fb {
  ($header:expr, $($arg:tt)*) => ({
    use core::fmt::Write;
    use $crate::logging::FRAMEBUFFER;
    let mut framebuffer = FRAMEBUFFER.lock();
    framebuffer.write_fmt(format_args!("{}: ", $header)).unwrap();
    framebuffer.write_fmt(format_args!($($arg)*)).unwrap();
    framebuffer.write_str("\r\n").unwrap();
  });
}

#[macro_export]
macro_rules! debug {
  ($($arg:tt)*) => {
    $crate::log_dbg!("DEBUG", $($arg)*);
    $crate::log_fb!("DEBUG", $($arg)*);
  };
}

#[macro_export]
macro_rules! info {
  ($($arg:tt)*) => {
    $crate::log_dbg!("INFO", $($arg)*);
    $crate::log_fb!("INFO", $($arg)*);
  };
}

#[macro_export]
macro_rules! warn {
  ($($arg:tt)*) => {
    $crate::log_dbg!("WARN", $($arg)*);
    $crate::log_fb!("WARN", $($arg)*);
  };
}

#[macro_export]
macro_rules! error {
  ($($arg:tt)*) => {
    $crate::log_dbg!("ERROR", $($arg)*);
    $crate::log_fb!("ERROR", $($arg)*);
  };
}
