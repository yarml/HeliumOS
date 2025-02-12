use {
  crate::sync::{NaiveRwLock, NaiveRwLockReadGuard, NaiveRwLockWriteGuard},
  core::{
    mem,
    sync::atomic::{
      AtomicI128, AtomicI16, AtomicI32, AtomicI64, AtomicI8, AtomicIsize,
      AtomicU128, AtomicU16, AtomicU32, AtomicU64, AtomicU8, AtomicUsize,
      Ordering,
    },
  },
};

macro_rules! expose_inner {
  {
    $($name:ident ( $($param_name:ident : $param_ty:ty),* ) $(-> $return_ty:ty)?);* $(;)?
  } => {
    $(pub fn $name(&self, index: usize, $($param_name:$param_ty),*) $(-> $return_ty)* {
      let guard = self.data.read();
      guard[index].$name($($param_name),*)
    })*
  };
}

macro_rules! expose_fetch {
    {
      $int_type:ty => ($($name:ident),* $(,)?)
    } => {
        expose_inner! {
          $($name(val: $int_type, ordering: Ordering) -> $int_type);*
        }
    };
}

macro_rules! make_atomic_array {
  ($atomic_array_type:ident,$atomic_type:ident,$int_type:ident) => {
    pub struct $atomic_array_type<const N: usize> {
      data: NaiveRwLock<[$atomic_type; N]>,
    }

    impl<const N: usize> $atomic_array_type<N> {
      pub const fn new(value: [$int_type; N]) -> Self {
        let data = unsafe {
          // # Safety
          // AtomicType and IntType have the same underlying representation
          mem::transmute_copy(&value)
        };
        Self {
          data: NaiveRwLock::new(data),
        }
      }
      pub const fn new_atomic(value: [$atomic_type; N]) -> Self {
        Self {
          data: NaiveRwLock::new(value),
        }
      }
    }
    impl<const N: usize> $atomic_array_type<N> {
      pub const fn len(&self) -> usize {
        N
      }
      pub fn iter(&self) -> AtomicArrayIter<'_, $atomic_type, N> {
        let guard = self.data.read();
        AtomicArrayIter::new(guard)
      }
    }
    impl<const N: usize> $atomic_array_type<N> {
      expose_fetch! {
        $int_type => (
          fetch_add,
          fetch_sub,
          fetch_and,
          fetch_or,
          fetch_xor,
          fetch_nand,
          fetch_max,
          fetch_min,
          swap
        )
      }
      expose_inner! {
        store(val: $int_type, order: Ordering);
        load(order: Ordering) -> $int_type;
        compare_exchange(
          current: $int_type,
          new: $int_type,
          success: Ordering,
          failure: Ordering
        ) -> Result<$int_type, $int_type>;
        compare_exchange_weak(
          current: $int_type,
          new: $int_type,
          success: Ordering,
          failure: Ordering
        ) -> Result<$int_type, $int_type>;
      }
      pub fn fetch_update<F>(
        &self,
        index: usize,
        set_order: Ordering,
        fetch_order: Ordering,
        f: F,
      ) -> Result<$int_type, $int_type>
      where
        F: FnMut($int_type) -> Option<$int_type>,
      {
        let guard = self.data.read();
        guard[index].fetch_update(set_order, fetch_order, f)
      }
      pub fn write(&self) -> NaiveRwLockWriteGuard<'_, [$int_type; N]> {
        // Going through a deferred guard first will (hopefully) not starve writers
        let data = self.data.deferred_write().write();
        unsafe {
          // # Safety
          // AtomicType and IntType have the same bit representation
          data.reinterpret()
        }
      }
    }
    impl<const N: usize> Iterator for AtomicArrayIter<'_, $atomic_type, N> {
      type Item = $int_type;
      fn next(&mut self) -> Option<Self::Item> {
        if self.index >= self.len {
          None
        } else {
          Some(self.guard[self.index].load(Ordering::Relaxed))
        }
      }
    }
  };
}

make_atomic_array!(AtomicU8Array, AtomicU8, u8);
make_atomic_array!(AtomicI8Array, AtomicI8, i8);
make_atomic_array!(AtomicU16Array, AtomicU16, u16);
make_atomic_array!(AtomicI16Array, AtomicI16, i16);
make_atomic_array!(AtomicU32Array, AtomicU32, u32);
make_atomic_array!(AtomicI32Array, AtomicI32, i32);
make_atomic_array!(AtomicU64Array, AtomicU64, u64);
make_atomic_array!(AtomicI64Array, AtomicI64, i64);
make_atomic_array!(AtomicUsizeArray, AtomicUsize, usize);
make_atomic_array!(AtomicIsizeArray, AtomicIsize, isize);
make_atomic_array!(AtomicU128Array, AtomicU128, u128);
make_atomic_array!(AtomicI128Array, AtomicI128, i128);

pub struct AtomicArrayIter<'a, T: 'a, const N: usize> {
  guard: NaiveRwLockReadGuard<'a, [T; N]>,
  index: usize,
  len: usize,
}

impl<'a, T: 'a, const N: usize> AtomicArrayIter<'a, T, N> {
  pub fn new(guard: NaiveRwLockReadGuard<'a, [T; N]>) -> Self {
    let len = guard.len();
    Self {
      guard,
      index: 0,
      len,
    }
  }
}
