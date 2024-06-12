#![no_std]
#![no_main]

use core::panic::PanicInfo;

use helium_runtime::syscall::{self, debug_draw, getpid};

#[no_mangle]
extern "C" fn run() -> u64 {
  let pid = getpid();
  if pid < 10 {
    debug_draw(10 + pid as usize, 10, 255, 0, 0);
  }
  pid * 2
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
  syscall::exit(32);
}

