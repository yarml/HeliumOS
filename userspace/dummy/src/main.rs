#![no_std]
#![no_main]

use core::panic::PanicInfo;
use helium_runtime::syscall;

#[no_mangle]
extern "C" fn run() -> usize {
  loop {}
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
  syscall::exit(32);
}
