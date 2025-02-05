#[macro_export]
macro_rules! debug_check {
  ($cond:expr, $msg:expr) => {
    #[cfg(feature = "debug_checks")]
    if !$cond {
      panic!($msg);
    }
  };
}
