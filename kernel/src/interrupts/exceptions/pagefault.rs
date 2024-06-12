use x86_64::{
  addr::VirtAddrNotValid,
  registers::control::Cr2,
  structures::idt::{InterruptStackFrame, PageFaultErrorCode},
};

use crate::{interrupts::exceptions::prologue, mem::heap, println};

pub extern "x86-interrupt" fn page_fault(
  frame: InterruptStackFrame,
  ec: PageFaultErrorCode,
) {
  let adr = match Cr2::read() {
    Ok(adr) => adr,
    Err(VirtAddrNotValid(adr)) => {
      panic!("Tried using an invalid address: {adr:x}")
    }
  };

  // PageFault in kernel heap? Fear not, we will allocate it and return
  if !ec.contains(PageFaultErrorCode::USER_MODE) && heap::is_heap(adr) {    
    heap::expand(adr);
    return;
  }

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

  // For the far far far future, implement swapping here
  panic!();
}
