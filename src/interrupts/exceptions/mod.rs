use crate::println;

use x86_64::structures::idt::InterruptStackFrame;

pub mod div;
pub mod gprot;
pub mod pagefault;

fn prologue(frame: &InterruptStackFrame, exception: &str) {
  println!("[Proc ] [Exception: {}]", exception);
  println!("IP: {:?}", frame.instruction_pointer.as_ptr::<*const ()>());
  println!("SP: {:?}", frame.stack_pointer.as_ptr::<*const ()>());
  println!("CS: {:x}", frame.code_segment);
  println!("SS: {:x}", frame.stack_segment);
  println!("RF: {:x}", frame.cpu_flags);
}
