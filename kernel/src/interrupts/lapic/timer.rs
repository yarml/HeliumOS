use core::{
  sync::atomic::{AtomicUsize, Ordering},
  usize,
};

use crate::{
  dev::framebuffer::debug_set_pixel,
  println,
  proc::{
    self,
    apic::regmap::LocalApicRegisterMap,
    task::{self, TaskProcState},
  },
};
use x86_64::VirtAddr;

extern "C" {
  static timer_inter_1: VirtAddr;
}

pub(in crate::interrupts) fn timer_inter_1_adr() -> VirtAddr {
  VirtAddr::new((unsafe { &timer_inter_1 }) as *const VirtAddr as u64)
}

static COUNT: AtomicUsize = AtomicUsize::new(0);

#[no_mangle]
extern "C" fn timer_inter_2(proc_state: &TaskProcState) -> ! {
  LocalApicRegisterMap::get().timer_reset(usize::MAX);
  let rip = proc_state.rip;
  println!("Interrupted task at IP: {:x}", rip);
  task::tick(proc_state);
  if proc::is_primary() {
    let count = COUNT.fetch_add(1, Ordering::SeqCst);
    let period = count / 10 % 2 == 0;
    let color = if period { (0, 0, 255) } else { (255, 0, 0) };
    let offset = count % 10;
    debug_set_pixel(1 + offset, 50, color.into());
  }
  unsafe { task::continue_current(true) }
}
