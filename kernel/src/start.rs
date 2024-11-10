use core::hint;

#[no_mangle]
fn _start() -> ! {
  loop {
    hint::spin_loop();
  }
}
