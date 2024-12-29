mod glob;

use core::arch::x86_64::__cpuid;

pub use glob::GlobHartInfo;

use crate::bootboot::BootbootHeader;

static GLOB_HART_INFO: GlobHartInfo = GlobHartInfo::new();

pub fn glob_info() -> &'static GlobHartInfo {
  &GLOB_HART_INFO
}

pub fn id() -> usize {
  (unsafe {
    // # Safety
    // Nothing to worry about
    __cpuid(1)
  }
  .ebx
    >> 24) as usize
}

pub fn is_bootstrap() -> bool {
  let bspid = BootbootHeader::instance().bspid();
  id() == bspid
}
