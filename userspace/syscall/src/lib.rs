#![no_std]

use num_derive::FromPrimitive;

#[repr(u64)]
pub enum SyscallResult {
  Success,
  Invalid,
  Pid,
}

#[derive(FromPrimitive)]
#[repr(u64)]
pub enum Syscall {
  Exit,
  GetPid,
}
