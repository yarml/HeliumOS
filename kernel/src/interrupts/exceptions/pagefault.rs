use alloc::vec::Vec;
use x86_64::{
  addr::VirtAddrNotValid,
  registers::control::Cr2,
  structures::idt::{InterruptStackFrame, PageFaultErrorCode},
};

use crate::{
  interrupts::exceptions::prologue,
  mem::{early_heap::EarlyAllocator, heap},
  println,
};

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
  let operation = if ec.contains(PageFaultErrorCode::INSTRUCTION_FETCH) {
    "fetch"
  } else if ec.contains(PageFaultErrorCode::CAUSED_BY_WRITE) {
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

  // Not sure if the normal heap will be safe to use here
  let mut flags = Vec::new_in(EarlyAllocator);

  if ec.contains(PageFaultErrorCode::PROTECTION_KEY) {
    flags.push("protection-key violation");
  }
  if ec.contains(PageFaultErrorCode::SHADOW_STACK) {
    flags.push("shadow-stack violation");
  }
  if ec.contains(PageFaultErrorCode::SGX) {
    flags.push("SGX");
  }

  // For the far far far future, implement swapping here
  panic!(
    "Memory violation caused by {cause} while trying to {operation} at {:p
  } in {privilege} mode. Additional flags: {flags:?}",
    adr.as_ptr::<()>()
  );
}
