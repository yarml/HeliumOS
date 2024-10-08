mod exceptions;
mod lapic;
pub mod pic;

use self::{
  exceptions::{
    div::div, doublefault::doublefault, gprot::gprot, pagefault::page_fault,
    stacksegfault::stacksegfault,
  },
  lapic::{error::error, spurious::spurious, timer::timer_inter_1_adr},
};
use spin::RwLock;
use x86_64::structures::idt::{HandlerFunc, InterruptDescriptorTable};

pub const ERROR_STACK_SIZE: usize = 8 * 1024;

#[repr(u8)]
pub enum Vectors {
  TestInterrupt = 0x20,
  LocalApicError = 0xFD,
  LocalApicSpurious = 0xFF,
  LocalApicTimer = 0xF0,
}

impl From<Vectors> for u8 {
  fn from(value: Vectors) -> Self {
    value as u8
  }
}

static IDT: RwLock<InterruptDescriptorTable> =
  RwLock::new(InterruptDescriptorTable::new());
static CALIB_IDT: RwLock<InterruptDescriptorTable> =
  RwLock::new(InterruptDescriptorTable::new());

pub fn init() {
  let mut idt = IDT.write();

  idt.divide_error.set_handler_fn(div);
  idt.general_protection_fault.set_handler_fn(gprot);
  idt.page_fault.set_handler_fn(page_fault);
  idt.stack_segment_fault.set_handler_fn(stacksegfault);
  unsafe {
    idt
      .double_fault
      .set_handler_fn(doublefault)
      .set_stack_index(1)
  };

  idt[Vectors::LocalApicError as u8].set_handler_fn(error);
  idt[Vectors::LocalApicSpurious as u8].set_handler_fn(spurious);
  unsafe {
    idt[Vectors::LocalApicTimer as u8].set_handler_addr(timer_inter_1_adr())
  };
}

pub fn load() {
  let idt = IDT.read();
  unsafe { idt.load_unsafe() };
}

// Sets up IDT[0x20] to call handler
pub(super) fn setup_pit_calib(handler: HandlerFunc) {
  let mut idt = CALIB_IDT.write();
  idt[0x20].set_handler_fn(handler);
  unsafe { idt.load_unsafe() };
}
