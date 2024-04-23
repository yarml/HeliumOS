use core::arch::asm;
use helium_syscall::Syscall;

fn syscall1(syscall: Syscall, arg0: u64) {
  unsafe {
    asm! {
      "syscall",
      in("rax") syscall as u64,
      in("rdi") arg0,
    }
  }
}

pub fn exit(exitcode: u64) -> ! {
  syscall1(Syscall::Exit, exitcode);
  panic!("Exit failure");
}

