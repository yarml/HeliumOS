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
mod mem;
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

  println!("Disabling PIC");
  pic::disable();

  println!("Initializing memory");
  mem::init();

  println!("Loop");
  loop {}
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
  loop {}
}
