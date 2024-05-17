use crate::{mem::gdt::KernelGlobalDescriptorTable, sys};
use helium_syscall::{Syscall, SyscallResult};
use num_traits::FromPrimitive;
use x86_64::{
  registers::{
    control::{Efer, EferFlags},
    model_specific::{LStar, Star},
  },
  VirtAddr,
};

use super::task;

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
  _rsi: u64,
  _rdx: u64,
  _rcx: u64,
  _r8: u64,
  _r9: u64,
  rax: u64,
) -> SyscallResult {
  let syscall = match Syscall::from_u64(rax) {
    None => return SyscallResult::Invalid,
    Some(syscall) => syscall,
  };

  match syscall {
    Syscall::Exit => {
      // Move the current process to the graveyard where it can be cleaned up
      // later by the kernel
      // And set the current process to nothing, then halt.
      let exit_code = rdi as usize;
      task::exit_current(exit_code);
      sys::event_loop();
    }
    Syscall::GetPid => {
      // let pinfo = ProcInfo::instance();
      // let task = pinfo.current_task.as_ref().unwrap().read();
      // let pid = task.id;
      SyscallResult::Pid
    }
  }
}
