mod feature;

pub use feature::FeatureSet;

use crate::{bootboot::BootbootHeader, system::SystemStage};
use core::{arch::x86_64::__cpuid, hint};

pub struct ThisHart;

impl ThisHart {
  pub fn id() -> usize {
    match SystemStage::current() {
      SystemStage::Initialization => Self::id_cpuid(),
    }
  }

  pub fn is_bootstrap() -> bool {
    let bspid = match SystemStage::current() {
      SystemStage::Initialization => BootbootHeader::instance().bspid(),
    };
    Self::id() == bspid
  }

  pub fn die() -> ! {
    loop {
      hint::spin_loop();
    }
  }
}

impl ThisHart {
  fn id_cpuid() -> usize {
    (unsafe {
      // # Safety
      // Nothing to worry about
      __cpuid(1)
    }
    .ebx
      >> 24) as usize
  }
}
