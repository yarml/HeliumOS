use crate::{mem::gdt::KernelGlobalDescriptorTable, println};
use num_derive::FromPrimitive;
use num_traits::FromPrimitive;
use x86_64::{
  instructions::interrupts,
  registers::{
    control::{Efer, EferFlags},
    model_specific::{LStar, Star},
  },
  VirtAddr,
};

extern "C" {
  static syscall_handle_1: VirtAddr;
}

pub(super) fn enable() {
  let gdt = unsafe { KernelGlobalDescriptorTable::entry() };
  LStar::write(syscall1_adr());
  Star::write(gdt.ucode_seg, gdt.udata_seg, gdt.kcode_seg, gdt.kdata_seg)
    .unwrap();
  let efer = Efer::read();
  unsafe { Efer::write(efer | EferFlags::SYSTEM_CALL_EXTENSIONS) };
}

fn syscall1_adr() -> VirtAddr {
  VirtAddr::new((unsafe { &syscall_handle_1 }) as *const VirtAddr as u64)
}

#[no_mangle]
extern "C" fn syscall_handle_2(
  rdi: u64,
  rsi: u64,
  rdx: u64,
  rcx: u64,
  r8: u64,
  r9: u64,
  rax: u64,
) -> SyscallResult {
  println!("Syscall!");
  let syscall = match Syscall::from_u64(rax) {
    None => return SyscallResult::InvalidSyscall,
    Some(syscall) => syscall,
  };

  match syscall {
    Syscall::Write => println!("Write"),
    Syscall::Exit => {
      println!("Exit");
    }
  }

  SyscallResult::Sucess
}

#[repr(u64)]
enum SyscallResult {
  Sucess,
  InvalidSyscall,
  Error,
}

#[derive(FromPrimitive)]
#[repr(u64)]
enum Syscall {
  Write,
  Exit,
}
