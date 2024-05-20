#![no_std]
#![no_main]

use core::panic::PanicInfo;

use helium_runtime::syscall::{self, getpid};

#[no_mangle]
extern "C" fn run() -> u64 {
  getpid() * 2
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
  syscall::exit(32);
}

