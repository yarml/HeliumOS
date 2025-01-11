use core::{hint, panic::PanicInfo};

use crate::error;

#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
  if let Some(location) = info.location() {
    error!("Kernel Panic({}): {}", location, info.message());
  } else {
    error!("Kernel Panic: {}", info.message());
  }
  loop {
    hint::spin_loop();
  }
}
