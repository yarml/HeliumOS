use core::sync::atomic::{AtomicBool, Ordering};

use crate::{fs::initrd, println};

static READY: AtomicBool = AtomicBool::new(false);

pub fn ready() -> bool {
  READY.fetch_or(false, Ordering::SeqCst)
}

pub fn init() {
  let fontfile = initrd::open_file("/sys/font.psf");
  println!("file found: {}", fontfile.is_some());
}
