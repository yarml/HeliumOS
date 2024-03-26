use crate::proc::{apic::regmap::LocalApicRegisterMap, task};
use x86_64::structures::idt::InterruptStackFrame;

pub(in crate::interrupts) extern "x86-interrupt" fn timer(
  _frame: InterruptStackFrame,
) {
  task::tick();
  LocalApicRegisterMap::get().eoi();
}
