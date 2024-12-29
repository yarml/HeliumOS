use crate::{hart, mem};
use core::hint;

#[no_mangle]
fn _start() -> ! {
  if !hart::is_bootstrap() {
    loop {
      hint::spin_loop();
    }
  }
  mem::init();
  loop {
    hint::spin_loop();
  }
}
