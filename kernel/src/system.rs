use crate::sync::Mutex;

static SYSTEM_STAGE: Mutex<SystemStage> =
  Mutex::new(SystemStage::Initialization);

#[derive(Clone, Copy)]
pub enum SystemStage {
  Initialization,
}

impl SystemStage {
  pub fn current() -> Self {
    *SYSTEM_STAGE.lock()
  }
}
