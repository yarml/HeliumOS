use spin::Mutex;

use self::gdt::{gdt, gdt_null, GDTEntry};

pub mod gdt;

static BASIC_GDT: Mutex<[GDTEntry; 3]> =
  Mutex::new([gdt_null(), gdt_null(), gdt_null()]);

pub fn init() {
  {
    let mut basic_gdt_guard = BASIC_GDT.lock();
    let basic_gdt_writer = basic_gdt_guard.as_mut();
    basic_gdt_writer[1] = gdt(true, false, 0);
    basic_gdt_writer[2] = gdt(false, true, 0);
    gdt::load(basic_gdt_writer.as_ref(), 0x10, 0x08, 0x00);
  }
}
