use {
  crate::debug_check,
  core::{
    hash::Hash,
    mem::MaybeUninit,
    ops::{Deref, DerefMut, DerefPure, Index, IndexMut},
    slice::SliceIndex,
  },
};

pub struct SmallVec<T, const N: usize> {
  buffer: [MaybeUninit<T>; N],
  len: usize,
}

impl<T, const N: usize> SmallVec<T, N> {
  pub fn new() -> Self {
    Self {
      buffer: [const { MaybeUninit::uninit() }; N],
      len: 0,
    }
  }
}

impl<T, const N: usize> SmallVec<T, N> {
  #[must_use = "check that value was added, otherwise it will just drop"]
  pub fn push(&mut self, value: T) -> Result<&T, T> {
    debug_check!(
      self.len <= N,
      "SmallVec contains more elements than it's maximum capacity."
    );
    if self.len == N {
      return Err(value);
    }
    self.buffer[self.len] = MaybeUninit::new(value);
    let r = unsafe {
      // # Safety
      // Just made the sucker
      self.buffer[self.len].assume_init_ref()
    };
    self.len += 1;
    Ok(r)
  }

  pub fn pop(&mut self) -> Option<T> {
    if self.len == 0 {
      return None;
    }

    self.len -= 1;
    Some(unsafe {
      // # Safety
      // Value previously added since len indicates so
      // Move is fine since we promise not to give it again unless added back as next push
      self.buffer[self.len].assume_init_read()
    })
  }
  pub fn erase(&mut self, index: usize) -> Option<T> {
    if self.len <= index {
      return None;
    }
    self.buffer[index..self.len].rotate_left(1);
    self.pop()
  }
}

impl<T, const N: usize> Drop for SmallVec<T, N> {
  fn drop(&mut self) {
    for i in 0..self.len {
      unsafe {
        // # Safety
        // Value previously added since len indicates so
        self.buffer[i].assume_init_drop()
      };
    }
  }
}

impl<T, const N: usize> Deref for SmallVec<T, N> {
  type Target = [T];

  fn deref(&self) -> &Self::Target {
    unsafe {
      // # Safety
      // Values previously added since len indicates so
      MaybeUninit::slice_assume_init_ref(&self.buffer[..self.len])
    }
  }
}

impl<T, const N: usize> DerefMut for SmallVec<T, N> {
  fn deref_mut(&mut self) -> &mut Self::Target {
    unsafe {
      // # Safety
      // Values previously added since len indicates so
      MaybeUninit::slice_assume_init_mut(&mut self.buffer[..self.len])
    }
  }
}

/// # Safety
/// Since DerefPure is unstable this needs to be checked whenever the compiler is updated
/// For now, consecutive calls to Deref and DerefMut will always give the same value if no mutation
/// happens in between
unsafe impl<T, const N: usize> DerefPure for SmallVec<T, N> {}

impl<T, const N: usize, I: SliceIndex<[T]>> Index<I> for SmallVec<T, N> {
  type Output = I::Output;

  fn index(&self, index: I) -> &Self::Output {
    Index::index(&**self, index)
  }
}

impl<T, const N: usize, I: SliceIndex<[T]>> IndexMut<I> for SmallVec<T, N> {
  fn index_mut(&mut self, index: I) -> &mut Self::Output {
    IndexMut::index_mut(&mut **self, index)
  }
}

impl<T: Hash, const N: usize> Hash for SmallVec<T, N> {
  fn hash<H: core::hash::Hasher>(&self, state: &mut H) {
    self[..self.len].hash(state);
  }
}
