#[cfg(feature = "logger_framebuffer")]
mod fb;

#[cfg(feature = "logger_port_e9")]
mod port_e9;

#[doc(hidden)]
#[cfg(feature = "logger_port_e9")]
pub use port_e9::DEBUG_PORT;

#[doc(hidden)]
pub use fb::FRAMEBUFFER;

#[macro_export]
macro_rules! log {
  ($header:expr, $($arg:tt)*) => {
    $crate::log_dbg!($header, $($arg)*);
    $crate::log_fb!($header, $($arg)*);
  };
}

#[macro_export]
macro_rules! debug {
  ($($arg:tt)*) => {
    #[cfg(feature = "log_debug")]
    {
      $crate::log!("DEBUG", $($arg)*);
    }
  };
}

#[macro_export]
macro_rules! info {
  ($($arg:tt)*) => {
    #[cfg(feature = "log_info")]
    {
      $crate::log!("INFO", $($arg)*);
    }
  };
}

#[macro_export]
macro_rules! warn {
  ($($arg:tt)*) => {
    #[cfg(feature = "log_warn")]
    {
      $crate::log!("WARN", $($arg)*);
    }
  };
}

#[macro_export]
macro_rules! error {
  ($($arg:tt)*) => {
    #[cfg(feature = "log_error")]
    {
      $crate::log!("ERROR", $($arg)*);
    }
  };
}

/// This must never be used directly, and is only an implmentation detail
#[doc(hidden)]
#[macro_export]
macro_rules! log_routine {
  ($logger:expr, $header:expr, $($arg:tt)*) => {
    let id = *$crate::hart::ThisHart::id();
    $logger.write_fmt(format_args!("[Proc#{:02}] {:>5}: ", id, $header)).unwrap();
    $logger.write_fmt(format_args!($($arg)*)).unwrap();
    $logger.write_str("\r\n").unwrap();
  };
}

/// This must never be used directly, and is only an implmentation detail
#[doc(hidden)]
#[macro_export]
macro_rules! log_dbg {
  ($header:expr, $($arg:tt)*) => ({
    #[cfg(feature = "logger_port_e9")]
    {
      use core::fmt::Write;
      use $crate::logging::DEBUG_PORT;
      let mut debug_port = DEBUG_PORT.lock();
      $crate::log_routine!(debug_port, $header, $($arg)*);
    }
  });
}

/// This must never be used directly, and is only an implmentation detail
#[doc(hidden)]
#[macro_export]
macro_rules! log_fb {
  ($header:expr, $($arg:tt)*) => {{
    #[cfg(feature = "logger_framebuffer")]
    {
      use core::fmt::Write;
      use $crate::logging::FRAMEBUFFER;
      let mut framebuffer = FRAMEBUFFER.lock();
      $crate::log_routine!(framebuffer, $header, $($arg)*);
    }
  }};
}
