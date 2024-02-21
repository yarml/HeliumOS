use core::arch::asm;
use modular_bitfield::bitfield;
use modular_bitfield::specifiers::*;
use core::mem;

#[bitfield]
pub struct GDTEntry {
  limit0: B16,
  base0: B24,
  accessed: bool,
  write: bool,
  direction: bool,
  exec: bool,
  nsys: bool, // Must be 1
  dpl: B2,
  present: bool,
  limit1: B4,
  #[skip]
  __: B1,
  lmode: bool,
  size: bool,
  granularity: B1,
  base1: B8,
}

#[repr(C, packed)]
struct GDTReg {
  limit: u16,
  base: *const GDTEntry,
}

pub fn gdt(exec: bool, write: bool, dpl: u8) -> GDTEntry {
  GDTEntry::new()
    .with_nsys(true)
    .with_lmode(exec)
    .with_exec(exec)
    .with_write(write)
    .with_dpl(dpl)
    .with_present(true)
}
pub const fn gdt_null() -> GDTEntry {
  GDTEntry::new()
}

pub fn load(table: &[GDTEntry], data_sel: u16, code_sel: u16, tss_sel: u16) {
  let reg = GDTReg {
    limit: mem::size_of_val(table) as u16 - 1,
    base: table.as_ptr(),
  };

  unsafe {
    asm! {
      "lgdt [rdi]",
      "mov ss, ax",
      "mov ds, ax",
      "mov es, ax",
      "mov fs, ax",
      "mov gs, ax",
      "mov ax, cx",
      "ltr ax",
      "push rdx",
      "lea rdx, 42f",
      "push rdx",
      "retfq",
      "42:", // Far return location
      in("rdi") &reg,
      in("rax") data_sel,
      in("rdx") code_sel,
      in("rcx") tss_sel,
    }
  };
}
