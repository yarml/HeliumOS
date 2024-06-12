use helium_syscall::{Syscall, SyscallResult};

pub fn exit(exitcode: u64) -> ! {
  Syscall::Exit(exitcode).call();
  panic!("Exit failure"); // imagine failing exit, not as bad as failing linear algebra :'(
}

pub fn getpid() -> u64 {
  match Syscall::GetPid.call() {
    SyscallResult::Success(pid, _, _, _, _, _) => pid,
    _ => unreachable!(),
  }
}

pub fn debug_draw(x: usize, y: usize, r: u8, g: u8, b: u8) {
  Syscall::DebugDraw(x as u64, y as u64, r as u64, g as u64, b as u64).call();
}
