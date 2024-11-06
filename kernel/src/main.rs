#![no_std]
#![no_main]

extern crate alloc;

mod mem;
mod sys;

#[no_mangle]
fn _start() -> ! {
  loop {}
}
