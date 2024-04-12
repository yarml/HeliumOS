use x86_64::structures::idt::InterruptStackFrame;

use super::prologue;

pub extern "x86-interrupt" fn div(frame: InterruptStackFrame) {
  prologue(&frame, "Division Error");
  panic!();
}
