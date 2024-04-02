use x86_64::structures::idt::InterruptStackFrame;

use crate::println;

use super::prologue;

pub extern "x86-interrupt" fn stacksegfault(
  frame: InterruptStackFrame,
  ec: u64,
) {
  prologue(&frame, "StackSegment Fault");
  println!("Error code: {:08x}", ec);
  panic!();
}
