use x86_64::VirtAddr;

use crate::proc::{
  apic::regmap::LocalApicRegisterMap,
  task::{self, TaskProcState},
};

extern "C" {
  static timer_inter_1: VirtAddr;
}

pub(in crate::interrupts) fn timer_inter_1_adr() -> VirtAddr {
  VirtAddr::new((unsafe { &timer_inter_1 }) as *const VirtAddr as u64)
}

#[no_mangle]
extern "C" fn timer_inter_2(proc_state: &TaskProcState) {
  task::tick(proc_state);
  // Shouldn't do this all ticks, what usually happens in 1 process tick takes 3-4 process ticks
  // A simple program exiting with a code takes 3-4 process ticks
  // Framebuffer::instance().refresh();
  LocalApicRegisterMap::get().eoi();
  unsafe { task::continue_current() };
}
