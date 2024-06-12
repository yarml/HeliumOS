use core::{arch::asm, panic::PanicInfo};

use x86_64::instructions::{hlt, interrupts};

use crate::{dev::framebuffer::debug_set_pixel, println, proc::apic};

#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
  interrupts::disable();
  println!("[Proc {}] {}", apic::id(), info);
  debug_set_pixel(100, 100, (255, 0, 0).into());
  debug_set_pixel(101 + apic::id(), 100, (255, 0, 0).into());
  loop {
    hlt();
  }
}

pub fn pause() {
  unsafe {
    asm! {
      "pause"
    }
  }
}

pub fn event_loop() -> ! {
  unsafe {
    asm! {
      "sti",
      "2:",
      "hlt",
      "jmp 2b"
    }
  }
  unreachable!()
}
