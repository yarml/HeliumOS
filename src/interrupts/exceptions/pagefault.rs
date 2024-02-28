use super::prologue;
use crate::println;
use x86_64::{
  registers::control::Cr2,
  structures::idt::{InterruptStackFrame, PageFaultErrorCode},
};

pub extern "x86-interrupt" fn page_fault(
  frame: InterruptStackFrame,
  ec: PageFaultErrorCode,
) {
  let adr = Cr2::read();
  prologue(&frame, "Page Fault");
  let operation = if ec.contains(PageFaultErrorCode::CAUSED_BY_WRITE) {
    "write"
  } else {
    "read"
  };
  let privilege = if ec.contains(PageFaultErrorCode::USER_MODE) {
    "user"
  } else {
    "kernel"
  };
  let cause = if ec.contains(!PageFaultErrorCode::PROTECTION_VIOLATION) {
    "unset present flag"
  } else if ec.contains(PageFaultErrorCode::MALFORMED_TABLE) {
    "malformed table"
  } else {
    "page level protection"
  };

  println!(
    "Memory violation while trying to {} in {} mode",
    operation, privilege
  );
  println!("Violation happened at {:?}", adr.as_ptr::<*const ()>());
  println!("Vioation caused by {}", cause);

  if ec.contains(PageFaultErrorCode::PROTECTION_KEY) {
    println!("Caused by protection-key violation.");
  }
  if ec.contains(PageFaultErrorCode::SHADOW_STACK) {
    println!("Caused by shadow stack.");
  }
  if ec.contains(PageFaultErrorCode::INSTRUCTION_FETCH) {
    println!("While trying to fetch instruction.");
  }
  if ec.contains(PageFaultErrorCode::SGX) {
    println!("SGX.");
  }

  // For the far far far far far far far future, implement swapping here
  panic!();
}
