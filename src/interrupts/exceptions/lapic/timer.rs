use crate::{println, proc::apic::regmap::LocalApicRegisterMap};

use x86_64::structures::idt::InterruptStackFrame;

pub(in crate::interrupts) extern "x86-interrupt" fn timer(
  _frame: InterruptStackFrame,
) {
  println!("Hello");

  LocalApicRegisterMap::get().eoi();
}
