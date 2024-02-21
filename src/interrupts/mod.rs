pub mod pic;

use core::arch::asm;

pub fn disable() {
  unsafe {
    asm! {
      "cli"
    }
  }
}
