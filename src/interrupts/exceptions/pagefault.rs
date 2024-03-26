use x86_64::{
  addr::VirtAddrNotValid,
  registers::control::Cr2,
  structures::{
    idt::{InterruptStackFrame, PageFaultErrorCode},
    paging::{Page, PageTableFlags, Size4KiB},
  },
};

use crate::{
  interrupts::exceptions::prologue,
  mem::{self, heap, valloc},
  println,
  proc::apic,
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

  println!("PageFault");

  // PageFault in kernel heap? Fear not, we will allocate it and return
  let heap_end = heap::START.start_address() + heap::SIZE as u64;
  if heap::START.start_address() <= adr && adr < heap_end {
    let adr = Page::<Size4KiB>::containing_address(adr).start_address();
    let left = (heap_end - heap::START.start_address()) as usize;
    let expand = if left > heap::FRAG_SIZE {
      heap::FRAG_SIZE
    } else {
      left
    };

    println!(
      "[Proc {}] Expanding kernel heap by {} bytes from {:?}",
      apic::id(),
      expand,
      adr.as_ptr::<()>()
    );

    valloc(
      Page::from_start_address(adr).unwrap(),
      expand / mem::PAGE_SIZE,
      PageTableFlags::WRITABLE,
    );
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

  // For the far far far far far far far future, implement swapping here
  panic!();
}
