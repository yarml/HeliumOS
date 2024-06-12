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

use crate::{debug::rdtsc, dev::framebuffer::debug_set_pixel, fs::initrd, interrupts::pic};
use x86_64::instructions::interrupts as x86interrupts;

#[no_mangle]
fn _start() -> ! {
  x86interrupts::disable();
  {
    if !proc::is_primary() {
      proc::init::wait(); // Wait for initialization
    }
  }
  let start_time = debug::rdtsc();
  if debug::isvm() {
    println!("Running in VM");
    debug_set_pixel(0, 10, (0, 0, 255).into());
  } else {
    println!("Running on hardware");
    debug_set_pixel(0, 10, (0, 255, 0).into());
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
  debug_set_pixel(20, 20, (0, 0, 255).into());

  let end_time = rdtsc();
  let routine_time = end_time - start_time;
  println!("Startup routine took {} cycles", routine_time);

  println!("Processor ignition.");
  proc::init::ignite() // ---> late_start() eventually
}

fn late_start() {
  // Called by BSPID only after ignition but before processors are waiting in the event loop
  debug_set_pixel(20, 40, (0, 0, 255).into());
  println!("Device Initialization.");
  dev::init();
}
