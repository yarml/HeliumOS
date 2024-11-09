use core::{
  cell::Cell,
  hint,
  ops::{Deref, DerefMut},
  sync::atomic::{AtomicBool, Ordering},
};

pub struct Mutex<T: ?Sized> {
  lock: AtomicBool,
  data: Cell<T>,
}

pub struct MutexGuard<'mutex, T: ?Sized> {
  mutex: &'mutex Mutex<T>,
}

unsafe impl<T: ?Sized + Send> Send for Mutex<T> {}
unsafe impl<T: ?Sized + Send> Sync for Mutex<T> {}

unsafe impl<T: ?Sized + Send> Send for MutexGuard<'_, T> {}
unsafe impl<T: ?Sized + Sync> Sync for MutexGuard<'_, T> {}

impl<T> Mutex<T> {
  pub const fn new(data: T) -> Self {
    Self {
      lock: AtomicBool::new(false),
      data: Cell::new(data),
    }
  }
}

impl<T: ?Sized> Mutex<T> {
  pub fn lock(&self) -> MutexGuard<T> {
    loop {
      if let Some(guard) = self.try_lock() {
        return guard;
      }

      while self.is_locked() {
        hint::spin_loop();
      }
    }
  }

  pub fn try_lock(&self) -> Option<MutexGuard<T>> {
    if self
      .lock
      .compare_exchange(false, true, Ordering::Acquire, Ordering::Relaxed)
      .is_ok()
    {
      return Some(MutexGuard { mutex: &self });
    } else {
      None
    }
  }

  pub fn is_locked(&self) -> bool {
    self.lock.load(Ordering::Relaxed)
  }
}

impl<'lock, T: ?Sized> Drop for MutexGuard<'lock, T> {
  fn drop(&mut self) {
    self.mutex.lock.store(false, Ordering::Release);
  }
}

impl<'lock, T> Deref for MutexGuard<'lock, T> {
  type Target = T;

  fn deref(&self) -> &Self::Target {
    unsafe {
      // Safety: MutexGuard ensures exclusivity
      &*self.mutex.data.as_ptr()
    }
  }
}

impl<'lock, T> DerefMut for MutexGuard<'lock, T> {
  fn deref_mut(&mut self) -> &mut Self::Target {
    unsafe {
      // Safety: MutexGuard ensures exclusivity
      &mut *self.mutex.data.as_ptr()
    }
  }
}
