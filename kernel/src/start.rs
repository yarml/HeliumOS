use crate::mem;
use core::hint;

#[no_mangle]
fn _start() -> ! {
  mem::init();
  loop {
    hint::spin_loop();
  }
}
