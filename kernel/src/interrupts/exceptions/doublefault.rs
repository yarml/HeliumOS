use x86_64::structures::idt::InterruptStackFrame;

use crate::{interrupts::exceptions::prologue, println};

pub extern "x86-interrupt" fn doublefault(
  frame: InterruptStackFrame,
  ec: u64,
) -> ! {
  prologue(&frame, "Double Fault");
  println!("Error code: {:08x}", ec);
  panic!()
}
