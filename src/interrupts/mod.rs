mod exceptions;
pub mod pic;

use self::exceptions::{div::div, gprot::gprot, pagefault::page_fault};
use spin::RwLock;
use x86_64::structures::idt::InterruptDescriptorTable;

pub const ERROR_STACK_SIZE: usize = 2 * 1024;

static IDT: RwLock<InterruptDescriptorTable> =
  RwLock::new(InterruptDescriptorTable::new());

pub fn init() {
  let mut idt = IDT.write();

  idt.divide_error.set_handler_fn(div);
  idt.general_protection_fault.set_handler_fn(gprot);
  idt.page_fault.set_handler_fn(page_fault);
}

pub fn load() {
  let idt = IDT.read();
  unsafe { idt.load_unsafe() };
}
