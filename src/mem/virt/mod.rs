use x86_64::VirtAddr;

mod vcache;

pub const KVMSPACE: VirtAddr = VirtAddr::new_truncate(0xFFFF800000000000);

pub fn init() {
  vcache::init();
}
