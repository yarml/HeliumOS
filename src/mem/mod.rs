mod early_heap;
#[allow(dead_code)]
mod gdt;
mod heap;
mod phys;
pub mod virt;

pub const PAGE_SIZE: usize = 0x1000;

pub fn init() {
  gdt::basic_init();
  early_heap::init();
  phys::init();
  virt::init();
  heap::init();
}
