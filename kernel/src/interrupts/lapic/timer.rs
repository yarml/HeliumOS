use core::sync::atomic::{AtomicBool, Ordering};

use crate::{
  dev::framebuffer::debug_set_pixel,
  println,
  proc::{apic::regmap::LocalApicRegisterMap, task::TaskProcState},
  sys,
};
use x86_64::VirtAddr;

extern "C" {
  static timer_inter_1: VirtAddr;
}

pub(in crate::interrupts) fn timer_inter_1_adr() -> VirtAddr {
  VirtAddr::new((unsafe { &timer_inter_1 }) as *const VirtAddr as u64)
}

static PERIOD: AtomicBool = AtomicBool::new(false);

#[no_mangle]
extern "C" fn timer_inter_2(_proc_state: &TaskProcState) {
  //task::tick(proc_state);
  println!("Hello");
  let period = PERIOD.fetch_or(false, Ordering::SeqCst);
  PERIOD.store(!period, Ordering::SeqCst);
  if period {
    debug_set_pixel(200, 200, (255, 255, 0).into());
  } else {
    debug_set_pixel(200, 200, (0, 255, 255).into());
  }
  LocalApicRegisterMap::get().eoi();
  sys::event_loop()
  //unsafe { task::continue_current() };
}
