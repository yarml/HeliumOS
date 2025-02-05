mod feature;

use core::ops::Deref;

pub use feature::FeatureSet;

use {
  crate::{bootboot::BootbootHeader, system::SystemStage},
  core::{arch::x86_64::__cpuid, hint},
};

pub struct ThisHart;

#[derive(PartialEq, Eq)]
pub struct HartId(usize);

impl !Send for HartId {}
impl !Sync for HartId {}

impl ThisHart {
  pub fn id() -> HartId {
    match SystemStage::current() {
      SystemStage::Initialization => Self::id_cpuid(),
    }
  }

  pub fn is_bootstrap() -> bool {
    let bspid = match SystemStage::current() {
      SystemStage::Initialization => BootbootHeader::instance().bspid(),
    };
    *Self::id() == bspid
  }

  pub fn die() -> ! {
    loop {
      hint::spin_loop();
    }
  }
}

impl ThisHart {
  fn id_cpuid() -> HartId {
    HartId(
      (unsafe {
        // # Safety
        // Nothing to worry about
        __cpuid(1)
      }
      .ebx
        >> 24) as usize,
    )
  }
}

impl Deref for HartId {
  type Target = usize;

  fn deref(&self) -> &Self::Target {
    &self.0
  }
}
