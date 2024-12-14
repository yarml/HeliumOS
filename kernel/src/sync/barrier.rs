use crate::bootboot::BootbootHeader;
use core::{
  hint,
  sync::atomic::{AtomicBool, AtomicUsize, Ordering},
};

pub struct Barrier {
  waiting: AtomicUsize,
  done: AtomicBool,
  reset: usize,
}

enum State {
  Waiting,
  Leaving,
}

impl Barrier {
  #[inline]
  pub const fn new() -> Self {
    let numcores = BootbootHeader::instance().numcores();
    Self::with_count(numcores)
  }

  pub const fn with_count(count: usize) -> Self {
    Self {
      waiting: AtomicUsize::new(0),
      done: AtomicBool::new(false),
      reset: if count == 0 { 1 } else { count },
    }
  }
}

impl Barrier {
  pub fn sync(&self) {
    while self.done.load(Ordering::Acquire) {
      hint::spin_loop();
    }

    let prior = self.waiting.fetch_add(1, Ordering::AcqRel);
    if prior == self.reset - 1 {
      // Master, comes last, goes last, benevolent master
      self.done.store(true, Ordering::Release);
      while self
        .waiting
        .compare_exchange(1, 0, Ordering::Acquire, Ordering::Relaxed)
        .is_err()
      {
        hint::spin_loop();
      }
      self.done.store(false, Ordering::Release);
    } else {
      while !self.done.load(Ordering::Acquire) {
        hint::spin_loop();
      }
      self.waiting.fetch_sub(1, Ordering::SeqCst);
    }
  }
}
