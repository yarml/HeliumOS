#![no_std]

use core::arch::asm;

use ordinalizer::Ordinal;

#[derive(Ordinal)]
pub enum SyscallResult {
  Success(u64, u64, u64, u64, u64, u64),
  Invalid,
}

#[derive(Ordinal)]
pub enum Syscall {
  Exit(u64),
  GetPid,
  DebugDraw(u64, u64, u64, u64, u64), // x, y, r, g, b
}

impl Syscall {
  pub fn call(&self) -> SyscallResult {
    let num = self.ordinal();
    let (mut io0, mut io1, mut io2, mut io3, mut io4, mut io5): (u64, u64, u64, u64, u64, u64) = match self {
      Syscall::Exit(exitcode) => {
        (*exitcode, 0, 0, 0, 0, 0)
      },
      Syscall::GetPid => {
        (0, 0, 0, 0, 0, 0)
      },
      Self::DebugDraw(x, y, r, g, b) => {
        (*x, *y, *r, *g, *b, 0)
      }
    };

    let mut status: u64;
    unsafe {
      asm! {
        "syscall",
        inout("rax") num => status,
        inout("rdi") io0,
        inout("rsi") io1,
        inout("rdx") io2,
        inout("r10") io3,
        inout("r8") io4,
        inout("r9") io5,
      }
    }
    if status == 0 {
      SyscallResult::Success(io0, io1, io2, io3, io4, io5)
    } else {
      SyscallResult::Invalid
    }
  }
}
