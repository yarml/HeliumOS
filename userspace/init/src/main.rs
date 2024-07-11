#![no_std]
#![no_main]

use core::panic::PanicInfo;

use helium_runtime::syscall::{self, debug_draw, getpid};

const CANVAS_SIZE: usize = 20;
const INCREMENT: u8 = (255 / CANVAS_SIZE) as u8;

#[no_mangle]
extern "C" fn run() -> usize {
  let pid = getpid() as usize;
  if pid < CANVAS_SIZE {
    for i in 0..CANVAS_SIZE {
      debug_draw(pid as usize, 10 + i, INCREMENT * pid as u8, 255, INCREMENT * i as u8);
    }
  }
  pid * 2
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
  syscall::exit(32);
}

