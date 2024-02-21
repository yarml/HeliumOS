#![no_std]
#![no_main]

#[allow(dead_code)]
#[allow(non_snake_case)]
#[allow(non_camel_case_types)]
extern crate rlibc;
extern crate spin;

mod bootboot;
mod debug;
mod interrupts;
mod io;
mod proc;

use core::panic::PanicInfo;
use interrupts::pic;

#[no_mangle]
fn _start() -> ! {
  interrupts::disable();
  {
    if !proc::is_primary() {
      loop {}
    }
  }

  print!("Disabling PIC\n");
  pic::disable();

  print!("Loop\n");
  loop {}
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
  loop {}
}
