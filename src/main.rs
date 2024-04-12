#![no_std]
#![no_main]
#![feature(allocator_api)]
#![feature(alloc_layout_extra)]
#![feature(abi_x86_interrupt)]
#![feature(pointer_is_aligned)]
#![feature(ascii_char_variants)]
#![feature(ascii_char)]
#![feature(option_take_if)]

extern crate alloc;
extern crate core;
extern crate rlibc;
extern crate spin;

mod acpi;
#[allow(dead_code)]
mod bootboot;
pub mod cfgtb;
mod debug;
mod elf;
mod fs;
mod interrupts;
mod io;
mod mem;
mod proc;
mod sys;
mod utils;

use crate::{fs::initrd, interrupts::pic};
use x86_64::instructions::interrupts as x86interrupts;

#[no_mangle]
fn _start() -> ! {
  x86interrupts::disable();
  {
    if !proc::is_primary() {
      proc::init::wait(); // Wait for initialization
    }
  }

  println!("Disabling PIC");
  pic::disable();

  println!("Initializing memory");
  mem::init();

  println!("Initializing interrupt table.");
  interrupts::init();

  println!("Initializing INITRD.");
  initrd::init();

  println!("Initializing Config Tables.");
  cfgtb::init();

  println!("ACPI Lookup.");
  acpi::init();

  println!("Processor ignition.");
  proc::init::ignite()
}
