#![no_std]
#![no_main]
#![feature(allocator_api)]
#![feature(alloc_layout_extra)]

extern crate alloc;
extern crate core;
extern crate rlibc;
extern crate spin;

#[allow(dead_code)]
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
fn panic(info: &PanicInfo) -> ! {
  println!("{}", info);
  loop {}
}
