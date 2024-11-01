#![no_std]
#![no_main]
#![feature(allocator_api)]
#![feature(alloc_layout_extra)]
#![feature(abi_x86_interrupt)]
#![feature(ascii_char_variants)]
#![feature(ascii_char)]
#![feature(vec_push_within_capacity)]
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
mod dev;
mod elf;
mod fs;
mod interrupts;
mod io;
mod mem;
mod proc;
mod sys;
mod utils;

use crate::{fs::initrd, interrupts::pic};
use dev::{
  console,
  framebuffer::{self, debug_set_pixel},
};
use x86_64::instructions::interrupts as x86interrupts;

#[no_mangle]
fn _start() -> ! {
  x86interrupts::disable();
  {
    if !proc::is_primary() {
      proc::init::wait(); // Wait for initialization
    }
  }
  framebuffer::init();
  debug_set_pixel(0, 0, (255, 255, 255).into());
  if debug::isvm() {
    println!("Running in VM");
  } else {
    println!("Running on hardware");
  }
  println!("Disabling PIC");
  pic::disable();

  debug_set_pixel(0, 1, (255, 255, 0).into());
  println!("Initializing memory");
  mem::init();

  debug_set_pixel(0, 2, (0, 255, 255).into());

  println!("Initializing interrupt table.");
  interrupts::init();

  println!("Initializing INITRD.");
  initrd::init();

  println!("Initializing console");
  console::init();

  println!("Initializing Config Tables.");
  cfgtb::init();

  debug_set_pixel(0, 3, (0, 0, 255).into());
  println!("ACPI Lookup.");
  acpi::init();

  debug_set_pixel(0, 4, (255, 0, 0).into());
  println!("Processor ignition.");
  proc::init::ignite() // ---> late_start() eventually
}

fn late_start() {
  // Called by BSPID only after ignition but before processors are waiting in the event loop
  debug_set_pixel(0, 5, (255, 255, 255).into());
  println!("Device Initialization.");
  dev::init();
}
