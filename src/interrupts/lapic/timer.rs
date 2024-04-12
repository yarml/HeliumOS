use crate::proc::{apic::regmap::LocalApicRegisterMap, task};
use x86_64::{instructions::interrupts, structures::idt::InterruptStackFrame};

pub(in crate::interrupts) extern "x86-interrupt" fn timer(
  _frame: InterruptStackFrame,
) {
  interrupts::disable();
  task::tick();
  LocalApicRegisterMap::get().eoi();
  task::continue_current();
}
