use crate::{println, proc::apic};

use x86_64::structures::idt::InterruptStackFrame;

pub mod div;
pub mod doublefault;
pub mod gprot;
pub mod pagefault;
pub mod stacksegfault;

fn prologue(frame: &InterruptStackFrame, exception: &str) {
  println!("[Proc {}] [Exception: {}]", apic::id(), exception);
  println!("IP: {:?}", frame.instruction_pointer.as_ptr::<*const ()>());
  println!("SP: {:?}", frame.stack_pointer.as_ptr::<*const ()>());
  println!("CS: {:x}", frame.code_segment.0);
  println!("SS: {:x}", frame.stack_segment.0);
  println!("RF: {:x}", frame.cpu_flags);
}
