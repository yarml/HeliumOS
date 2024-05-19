use helium_syscall::Syscall;

pub fn exit(exitcode: u64) -> ! {
  Syscall::Exit(exitcode).call();
  panic!("Exit failure"); // imagine failing exit, not as bad as failing linear algebra :'(
}

pub fn getpid() {
  todo!()
}

