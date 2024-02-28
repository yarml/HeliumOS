use core::panic::PanicInfo;

use x86_64::instructions::{hlt, interrupts};

use crate::println;

#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
  println!("{}", info);

  interrupts::disable();
  loop {
    hlt();
  }
}
