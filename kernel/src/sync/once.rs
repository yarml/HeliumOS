use core::{cell::UnsafeCell, hint, mem::MaybeUninit, sync::atomic::Ordering};
use status::{AtomicStatus, Status};

pub struct Once<T> {
  status: AtomicStatus,
  data: UnsafeCell<MaybeUninit<T>>,
}

unsafe impl<T: Send> Send for Once<T> {}
unsafe impl<T: Send + Sync> Sync for Once<T> {}

impl<T> Once<T> {
  pub const fn new() -> Self {
    Self {
      status: AtomicStatus::new(Status::Uninit),
      data: UnsafeCell::new(MaybeUninit::uninit()),
    }
  }
}

impl<T> Once<T> {
  pub fn init<F>(&self, data_fn: F) -> bool
  where
    F: FnOnce() -> T,
  {
    if self
      .status
      .compare_exchange(
        Status::Uninit,
        Status::Running,
        Ordering::Acquire,
        Ordering::Acquire,
      )
      .is_ok()
    {
      let data = data_fn();
      *unsafe {
        // # Safety:
        // UnsafeCell pointer guarentees most "Pointer to reference conversion"
        // self.status() == Runnin ensures we are the only current reference
        &mut *self.data.get()
      } = MaybeUninit::new(data);
      self.status.store(Status::Init, Ordering::Release);
      true
    } else {
      false
    }
  }
}

impl<T> Once<T> {
  pub fn get(&self) -> Option<&T> {
    if self.status() == Status::Init {
      Some(unsafe {
        // Safety: self.status() == Status::Init
        self.force_get()
      })
    } else {
      None
    }
  }

  pub fn poll(&self) -> Option<&T> {
    match self.status() {
      Status::Uninit => None,
      Status::Init => {
        Some(unsafe {
          // Safety: self.status() == Status::Init
          self.force_get()
        })
      }
      Status::Running => Some(self.wait()),
    }
  }

  pub fn wait(&self) -> &T {
    while self.status() != Status::Init {
      hint::spin_loop();
    }
    unsafe {
      // Safety: self.status() == Status::Init
      self.force_get()
    }
  }
}

impl<T> Once<T> {
  fn status(&self) -> Status {
    self.status.load(Ordering::Acquire)
  }

  /// # Safety
  /// Requires self.status() == Status::Init
  unsafe fn force_get(&self) -> &T {
    debug_assert_eq!(self.status(), Status::Init);
    let data = unsafe {
      // # Safety
      // Pointer from UnsafeCell guarentees most "convertible to a reference" conditions
      // self.status() == Status::Init ensures no aliasing &mut exists
      &*self.data.get()
    };
    unsafe {
      // # Safety
      // self.status() == Status::Init ensures the value is initialized
      data.assume_init_ref()
    }
  }
}

mod status {
  use core::{
    mem,
    sync::atomic::{AtomicU8, Ordering},
  };

  #[repr(u8)]
  #[derive(Debug, PartialEq)]
  pub enum Status {
    Uninit,
    Running,
    Init,
  }

  #[derive(Debug)]
  pub struct AtomicStatus {
    status: AtomicU8,
  }

  impl Status {
    /// # Safety
    /// Requires raw ∈ Status
    unsafe fn new_unchecked(raw: u8) -> Self {
      unsafe {
        // Safety: Ensured by caller
        mem::transmute(raw)
      }
    }
  }

  impl AtomicStatus {
    pub const fn new(status: Status) -> Self {
      Self {
        status: AtomicU8::new(status as u8),
      }
    }
  }

  impl AtomicStatus {
    pub fn compare_exchange(
      &self,
      current: Status,
      new: Status,
      success: Ordering,
      failure: Ordering,
    ) -> Result<u8, u8> {
      self
        .status
        .compare_exchange(current as u8, new as u8, success, failure)
    }

    pub fn load(&self, order: Ordering) -> Status {
      unsafe {
        // Safety: status always stores a copy of a valid Status
        Status::new_unchecked(self.status.load(order))
      }
    }

    pub fn store(&self, status: Status, order: Ordering) {
      self.status.store(status as u8, order)
    }
  }
}
