use core::arch::asm;

pub fn outb(adr: u16, data: u8) {
  unsafe {
    asm! {
      "out dx, al",
      in("rdx") adr,
      in("al") data,
    };
  };
}

pub fn outsb(adr: u16, data: &[u8]) {
  let len = data.len();
  let data = data.as_ptr();
  unsafe {
    asm! {
      "cld",
      "rep outs dx, BYTE PTR [rsi]",
      in("rcx") len,
      in("rsi") data,
      in("rdi") adr,
    };
  };
}

pub fn wait() {
  unsafe {
    asm! {
      "out 0x80, {0}",
      in(reg_byte) 0u8,
    };
  };
}
