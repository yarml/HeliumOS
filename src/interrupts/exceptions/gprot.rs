use crate::println;

use x86_64::structures::idt::InterruptStackFrame;

use super::prologue;

pub extern "x86-interrupt" fn gprot(frame: InterruptStackFrame, ec: u64) {
  prologue(&frame, "General Protection");

  println!("Error code: {:08x}", ec);

  panic!()
}
