use x86_64::structures::idt::InterruptStackFrame;

pub(in crate::interrupts) extern "x86-interrupt" fn spurious(
  _frame: InterruptStackFrame,
) {
}
