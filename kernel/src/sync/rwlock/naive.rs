use core::{
  cell::UnsafeCell,
  hint,
  ops::{Deref, DerefMut},
  sync::atomic::{AtomicUsize, Ordering},
};

type AtomicWord = AtomicUsize;
type Word = usize;
const BPW: usize = core::mem::size_of::<Word>();

pub struct RwLock<T: ?Sized> {
  lock: AtomicWord,
  data: UnsafeCell<T>,
}

pub struct RwLockReadGuard<'lock, T: 'lock + ?Sized> {
  lock: &'lock AtomicWord,
  data: &'lock T,
}

pub struct RwLockWriteGuard<'lock, T: 'lock + ?Sized> {
  lock: &'lock AtomicWord,
  data: &'lock mut T,
}

pub struct RwLockDeferredGuard<'lock, T: 'lock + ?Sized> {
  lock: &'lock AtomicWord,
  data: *mut T, // Keeping a mutable pointer to be able to change to writer, not keeping a &mut T to respect aliasing rules
}

// LSB 2 bits of lock stores the state
pub enum RwLockState {
  Open,
  ReadOnly,
  WriteOnly,
  Locked,
}

/// # Safety
/// Borrow checker will stop moves across thread boundaries if there is any reader
/// or writer. So T: Send should allow RwLock<T>: Send
unsafe impl<T: ?Sized + Send> Send for RwLock<T> {}

/// # Safety
/// Requiring T: Send in addition to T: Sync because RwLock can give mutable
/// references to T.
unsafe impl<T: ?Sized + Send + Sync> Sync for RwLock<T> {}

/// # Safety
/// RwLockReadGuard<T> is equivalent to &T
unsafe impl<T: ?Sized + Sync> Send for RwLockReadGuard<'_, T> {}
/// # Safety
/// RwLockReadGuard<T> is equivalent to &T
unsafe impl<T: ?Sized + Sync> Sync for RwLockReadGuard<'_, T> {}

/// # Safety
/// RwLockWriteuard<T> is equivalent to &mut T
/// &RwLockWriteuard<T> is equivalent to &&mut T, which is equivalent to &T
unsafe impl<T: ?Sized + Send + Sync> Send for RwLockWriteGuard<'_, T> {}
unsafe impl<T: ?Sized + Send + Sync> Sync for RwLockWriteGuard<'_, T> {}

/// # Safety
/// Alothough RwLockDeferredGuard is like RwLockReadGuard, it can be converted
/// to a RwLockWriteGuard, as such it has the same T: Send + Sync conditions.
unsafe impl<T: ?Sized + Send + Sync> Send for RwLockDeferredGuard<'_, T> {}
unsafe impl<T: ?Sized + Send + Sync> Sync for RwLockDeferredGuard<'_, T> {}

impl<T> RwLock<T> {
  pub const fn new(value: T) -> Self {
    Self {
      lock: AtomicUsize::new(0),
      data: UnsafeCell::new(value),
    }
  }
}

impl<T: ?Sized> RwLock<T> {
  pub fn reader_count(&self) -> Word {
    reader_count_from(self.lock.load(Ordering::Relaxed))
  }
  pub fn state(&self) -> RwLockState {
    // No panic: 2 bits taken always gives valid RwLockState
    state_from(self.lock.load(Ordering::Relaxed))
  }
}

impl<T: ?Sized> RwLock<T> {
  pub fn try_read(&self) -> Option<RwLockReadGuard<'_, T>> {
    if self
      .lock
      .fetch_update(Ordering::Acquire, Ordering::Relaxed, |lock| {
        let state = state_from(lock);
        let readers = reader_count_from(lock);
        if matches!(state, RwLockState::Open | RwLockState::ReadOnly) {
          Some(make_lock(state, readers + 1))
        } else {
          None
        }
      })
      .is_ok()
    {
      let data = unsafe {
        // # Safety
        // Pointer is valid since we host the data
        // Aliasing rules are checked on runtime
        &*self.data.get()
      };
      Some(RwLockReadGuard {
        lock: &self.lock,
        data,
      })
    } else {
      None
    }
  }
  pub fn try_write(&self) -> Option<RwLockWriteGuard<'_, T>> {
    if self
      .lock
      .fetch_update(Ordering::Acquire, Ordering::Relaxed, |lock| {
        let state = state_from(lock);
        let readers = reader_count_from(lock);
        if matches!(state, RwLockState::Open | RwLockState::WriteOnly)
          && readers == 0
        {
          Some(make_lock(RwLockState::Locked, 0))
        } else {
          None
        }
      })
      .is_ok()
    {
      let data = unsafe {
        // # Safety
        // Pointer is valid since we host the data
        // Aliasing rules are checked on runtime
        &mut *self.data.get()
      };
      Some(RwLockWriteGuard {
        lock: &self.lock,
        data,
      })
    } else {
      None
    }
  }
  pub fn try_deferred_write(&self) -> Option<RwLockDeferredGuard<'_, T>> {
    if self
      .lock
      .fetch_update(Ordering::Acquire, Ordering::Relaxed, |lock| {
        let state = state_from(lock);
        let readers = reader_count_from(lock);
        if matches!(state, RwLockState::Open) {
          Some(make_lock(RwLockState::WriteOnly, readers + 1))
        } else {
          None
        }
      })
      .is_ok()
    {
      let data = self.data.get();
      Some(RwLockDeferredGuard {
        lock: &self.lock,
        data,
      })
    } else {
      None
    }
  }
}

impl<T: ?Sized> RwLock<T> {
  pub fn read(&self) -> RwLockReadGuard<'_, T> {
    loop {
      if let Some(guard) = self.try_read() {
        return guard;
      }
      hint::spin_loop();
    }
  }
  pub fn write(&self) -> RwLockWriteGuard<'_, T> {
    loop {
      if let Some(guard) = self.try_write() {
        return guard;
      }
      hint::spin_loop();
    }
  }
  pub fn deferred_write(&self) -> RwLockDeferredGuard<'_, T> {
    loop {
      if let Some(guard) = self.try_deferred_write() {
        return guard;
      }
      hint::spin_loop();
    }
  }
}

impl<'lock, T: ?Sized> RwLockDeferredGuard<'lock, T> {
  pub fn try_write(
    self,
  ) -> Result<RwLockWriteGuard<'lock, T>, RwLockDeferredGuard<'lock, T>> {
    if self
      .lock
      .fetch_update(Ordering::Acquire, Ordering::Relaxed, |lock| {
        let readers = reader_count_from(lock);
        if readers == 0 {
          Some(make_lock(RwLockState::Locked, 0))
        } else {
          None
        }
      })
      .is_ok()
    {
      let data = unsafe { &mut *self.data };
      let lock = self.lock;
      core::mem::forget(self);
      Ok(RwLockWriteGuard { lock, data })
    } else {
      Err(self)
    }
  }
}

impl<'lock, T: ?Sized> RwLockDeferredGuard<'lock, T> {
  pub fn write(mut self) -> RwLockWriteGuard<'lock, T> {
    loop {
      match self.try_write() {
        Ok(write_guard) => return write_guard,
        Err(deferred_guard) => self = deferred_guard,
      }
      hint::spin_loop();
    }
  }
}

impl<'lock, T: ?Sized> RwLockWriteGuard<'lock, T> {
  pub fn read(self) -> RwLockReadGuard<'lock, T> {
    self
      .lock
      .fetch_update(Ordering::Acquire, Ordering::Relaxed, |_| {
        Some(make_lock(RwLockState::Open, 1))
      })
      .unwrap();
    let lock = self.lock;
    let data = unsafe {
      // # Safety
      // No write guard can race a mutable access to T since we already set the reader count to 1
      &*(self.data as *const T)
    };
    core::mem::forget(self);
    RwLockReadGuard { lock, data }
  }
  // Wouldn't make sense to convert a write guard to a deferred guard, since it will
  // be an exclusive reader inhibiting any further readers from entering the lock
  // The write guard already displays that behaviour.
}

impl<T: ?Sized> Deref for RwLockReadGuard<'_, T> {
  type Target = T;

  fn deref(&self) -> &Self::Target {
    self.data
  }
}

impl<T: ?Sized> Deref for RwLockWriteGuard<'_, T> {
  type Target = T;

  fn deref(&self) -> &Self::Target {
    self.data
  }
}

impl<T: ?Sized> DerefMut for RwLockWriteGuard<'_, T> {
  fn deref_mut(&mut self) -> &mut Self::Target {
    self.data
  }
}

impl<T: ?Sized> Deref for RwLockDeferredGuard<'_, T> {
  type Target = T;

  fn deref(&self) -> &Self::Target {
    unsafe {
      // # Safety
      // Data is hosted within a RwLock, so pointer is valid
      // Aliasing rules are runtime guarenteed with said lock
      &*self.data
    }
  }
}

impl<T: ?Sized> Drop for RwLockReadGuard<'_, T> {
  fn drop(&mut self) {
    self
      .lock
      .fetch_update(Ordering::Release, Ordering::Relaxed, |lock| {
        let state = state_from(lock);
        let readers = reader_count_from(lock);
        Some(make_lock(state, readers - 1))
      })
      .unwrap();
  }
}

impl<T: ?Sized> Drop for RwLockWriteGuard<'_, T> {
  fn drop(&mut self) {
    self
      .lock
      .fetch_update(Ordering::Release, Ordering::Relaxed, |_| {
        Some(make_lock(RwLockState::Open, 0))
      })
      .unwrap();
  }
}

impl<T: ?Sized> Drop for RwLockDeferredGuard<'_, T> {
  fn drop(&mut self) {
    self
      .lock
      .fetch_update(Ordering::Release, Ordering::Relaxed, |lock| {
        let readers = reader_count_from(lock);
        Some(make_lock(RwLockState::Open, readers - 1))
      })
      .unwrap();
  }
}

// TODO: make a proc macro for this
impl TryFrom<Word> for RwLockState {
  type Error = Word;

  fn try_from(value: Word) -> Result<Self, Self::Error> {
    match value {
      0 => Ok(Self::Open),
      1 => Ok(Self::ReadOnly),
      2 => Ok(Self::WriteOnly),
      3 => Ok(Self::Locked),
      other => Err(other),
    }
  }
}
impl From<RwLockState> for Word {
  fn from(value: RwLockState) -> Self {
    match value {
      RwLockState::Open => 0,
      RwLockState::ReadOnly => 1,
      RwLockState::WriteOnly => 2,
      RwLockState::Locked => 3,
    }
  }
}

fn state_from(lock: Word) -> RwLockState {
  RwLockState::try_from(lock & 0x3).unwrap()
}
const fn reader_count_from(lock: Word) -> Word {
  lock >> 2
}
fn make_lock(state: RwLockState, readers: Word) -> Word {
  Word::from(state) | readers << 2
}
