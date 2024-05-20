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

