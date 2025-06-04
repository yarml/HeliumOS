use core::str;

use crate::{
  dev::framebuffer::debug_set_pixel, mem::gdt::KernelGlobalDescriptorTable,
  println, sys,
};
use helium_syscall::{Syscall, SyscallResult};
use x86_64::{
  registers::{
    control::{Efer, EferFlags},
    model_specific::{LStar, Star},
  },
  VirtAddr,
};

use super::{
  task::{self, Task, TaskProcState, TASKS},
  ProcInfo,
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

trait SyscallHandler {
  fn handle(&self) -> SyscallResult;
}

impl SyscallHandler for Syscall {
  fn handle(&self) -> SyscallResult {
    match self {
      Syscall::Exit(exitcode) => {
        task::exit_current(*exitcode as usize);
        SyscallResult::Invalid // Does not matter
      }
      Syscall::GetPid => {
        println!("GetPid");
        let pinfo = ProcInfo::instance();
        let task = pinfo.current_task.as_ref().unwrap();
        let task = task.read();
        SyscallResult::Success(task.id as u64, 0, 0, 0, 0, 0)
      }
      Syscall::DebugDraw(x, y, r, g, b) => {
        println!("DebugDraw");
        debug_set_pixel(
          *x as usize,
          *y as usize,
          (*r as u8, *g as u8, *b as u8).into(),
        );
        SyscallResult::Success(0, 0, 0, 0, 0, 0)
      }
      Syscall::Spawn(path, len) => {
        println!("Spawn");
        let path = unsafe {
          // Trust userspace with my life
          str::from_raw_parts(*path, *len)
        };
        let mut tasks_lock = TASKS.get().unwrap().write();
        match Task::exec_initrd(path, &mut tasks_lock) {
          task::ExecResult::Success(id) => {
            SyscallResult::Success(id as u64, 0, 0, 0, 0, 0)
          }
          _ => SyscallResult::Failed,
        }
      }
    }
  }
}

fn syscall1_adr() -> VirtAddr {
  VirtAddr::new((unsafe { &syscall_handle_1 }) as *const VirtAddr as u64)
}

#[no_mangle]
extern "C" fn syscall_handle_2(proc_state: &TaskProcState) -> ! {
  // Everything here in it's own scope to ensure it
  // gets dropped before continue_current
  // What if compiler thinks it's smarted than me????
  {
    {
      // Update proc state
      let pinfo = ProcInfo::instance();
      let mut task = pinfo
        .current_task
        .as_ref()
        .expect("Syscall without running task?")
        .write();
      task.procstate = *proc_state;
    }
    let result = {
      let syscall = Syscall::try_from(proc_state);
      match syscall {
        Ok(syscall) => syscall.handle(),
        Err(_) => SyscallResult::Invalid,
      }
    };
    let pinfo = ProcInfo::instance();

    // If there is no current task, that means the syscall quit the task
    if pinfo.current_task.is_none() {
      sys::event_loop();
    }
    let current_task = pinfo.current_task.as_ref().unwrap();
    let mut task = current_task.write();
    let procstate = &mut task.procstate;
    match result {
      SyscallResult::Success(o0, o1, o2, o3, o4, o5) => {
        procstate.rax = 0;
        procstate.rdi = o0;
        procstate.rsi = o1;
        procstate.rdx = o2;
        procstate.r10 = o3;
        procstate.r8 = o4;
        procstate.r9 = o5;
      }
      _ => procstate.rax = 1,
    }
  }
  unsafe {
    task::continue_current(false);
  }
}

impl TryFrom<&TaskProcState> for Syscall {
  type Error = ();

  fn try_from(proc_state: &TaskProcState) -> Result<Self, Self::Error> {
    match proc_state.rax {
      // TODO: A proc macro will be good here
      0 => Ok(Syscall::Exit(proc_state.rdi)),
      1 => Ok(Syscall::GetPid),
      2 => Ok(Syscall::DebugDraw(
        proc_state.rdi,
        proc_state.rsi,
        proc_state.rdx,
        proc_state.r10,
        proc_state.r8,
      )),
      3 => Ok(Syscall::Spawn(
        proc_state.rdi as *const u8,
        proc_state.rsi as usize,
      )),
      _ => Err(()),
    }
  }
}
