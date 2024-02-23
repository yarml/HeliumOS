
mod early_heap;
#[allow(dead_code)]
pub mod gdt;
mod phys;
mod virt;

pub const PAGE_SIZE: usize = 0x1000;

pub fn init() {
  gdt::basic_init();
  early_heap::init();
  phys::init();
  virt::init();
}
