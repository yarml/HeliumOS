#![no_std]

use num_derive::FromPrimitive;

#[repr(u64)]
pub enum SyscallResult {
  Success,
  Invalid,
}

#[derive(FromPrimitive)]
#[repr(u64)]
pub enum Syscall {
  Write,
  Exit,
}
