mod port_e9;

#[doc(hidden)]
pub use port_e9::DEBUG_PORT;

/// This must never be used directly, and is only an implmentation detail
#[doc(hidden)]
#[macro_export]
macro_rules! log {
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
macro_rules! debug {
  ($($arg:tt)*) => {
    $crate::log!("DEBUG", $($arg)*)
  };
}

#[macro_export]
macro_rules! info {
  ($($arg:tt)*) => {
    $crate::log!("INFO", $($arg)*)
  };
}

#[macro_export]
macro_rules! warn {
  ($($arg:tt)*) => {
    $crate::log!("WARN", $($arg)*)
  };
}

#[macro_export]
macro_rules! error {
  ($($arg:tt)*) => {
    $crate::log!("ERROR", $($arg)*)
  };
}
