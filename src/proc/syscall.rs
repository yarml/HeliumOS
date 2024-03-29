use crate::mem::gdt::KernelGlobalDescriptorTable;
use x86_64::{
  registers::{
    control::{Efer, EferFlags},
    model_specific::{LStar, Star},
  },
  VirtAddr,
};

extern "C" {
  static syscall_handle: VirtAddr;
}

pub(super) fn enable() {
  let gdt = unsafe { KernelGlobalDescriptorTable::entry() };
  LStar::write(syscall_adr());
  Star::write(gdt.ucode_seg, gdt.udata_seg, gdt.kcode_seg, gdt.kdata_seg)
    .unwrap();
  let efer = Efer::read();
  unsafe { Efer::write(efer | EferFlags::SYSTEM_CALL_EXTENSIONS) };
}

fn syscall_adr() -> VirtAddr {
  VirtAddr::new((unsafe { &syscall_handle }) as *const VirtAddr as u64)
}
