use crate::{
  hart::{FeatureSet, ThisHart},
  info, mem,
};
use core::hint;

#[no_mangle]
fn _start() -> ! {
  if !ThisHart::is_bootstrap() {
    loop {
      hint::spin_loop();
    }
  }
  FeatureSet::ensure_enough();
  info!("All required features supported");

  mem::init();
  loop {
    hint::spin_loop();
  }
}
