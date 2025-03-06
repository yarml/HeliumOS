//! # Low Memory
//! Any usable memory in the range [1Mib 16Mib).
//! Not used for general purpose application,
//! Only for devices that require memory in that area
//!
//! # Supported Requests
//! - 64Kib frames on 64Kib boundaries
//! - 128Kib frames on 128Kib boundaries
//!
//! # Kernel Space
//! Like middle memory, this entire memory space can be found in kernel space
//! with constant offset using PhysAddr::to_virt()

use {
  super::{
    frame::{
      size::{Frame128KiB, Frame4KiB, Frame64KiB, FrameSize},
      Frame,
    },
    PhysAddr,
  },
  crate::sync::Mutex,
};

pub static LOWMEM_ALLOCATOR: Mutex<LowMemoryAllocator> =
  Mutex::new(LowMemoryAllocator::default());

pub struct LowMemoryAllocator {
  frames64: [usize; 4],
  frames128: [usize; 2],
}

impl LowMemoryAllocator {
  #[inline]
  pub const fn default() -> Self {
    Self {
      frames64: [usize::MAX; 4],
      frames128: [usize::MAX; 2],
    }
  }
}

impl LowMemoryAllocator {
  pub fn alloc128(&mut self) -> Option<Frame<Frame128KiB>> {
    let (word_index, word) = self
      .frames128
      .iter_mut()
      .enumerate()
      .filter(|(_, w)| **w != usize::MAX)
      .next()?;
    let bitindex = word.trailing_ones() as usize;
    *word |= 1 << bitindex;
    let frame_num = Self::frame_num(word_index, bitindex);
    Some(Self::frame(frame_num))
  }
  pub fn alloc64(&mut self) -> Option<Frame<Frame4KiB>> {
    // Prefer taking a frame whose buddy is used over using a new 128K frame
    // to minimize fragmentation
    for (word_index, word) in self
      .frames128
      .iter_mut()
      .enumerate()
      .filter(|(_, w)| **w != usize::MAX)
    {
      let mut accumulator = *word;
      while accumulator != usize::MAX {
        let bitindex = accumulator.trailing_ones() as usize;
        let buddy_bitindex = bitindex ^ 1;
        if (accumulator >> buddy_bitindex) & 1 == 1 {
          *word |= 1 << bitindex;
          let frame_num = Self::frame_num(word_index, bitindex);
          return Some(Self::frame(frame_num));
        }
        accumulator |= 1 << bitindex;
      }
    }

    // Not found, instead we allocate a 128K frame and take the first 64K in it
    let frame128 = self.alloc128()?;
    let num128 = frame128.number();
    let num64 = num128 << 1;
    self.mark_used64(num64);
    Some(Self::frame(num64))
  }

  pub fn free64(&mut self, frame: Frame<Frame64KiB>) {
    let num = frame.number();
    let buddy_num = Self::buddyof64(num);
    self.mark_free64(num);
    if self.isfree64(buddy_num) {
      self.mark_free128(Self::parentof64(num));
    }
  }
  pub fn free128(&mut self, frame: Frame<Frame128KiB>) {
    let num = frame.number();
    self.mark_free128(num);
  }
}

impl LowMemoryAllocator {
  #[inline]
  const fn mark_free64(&mut self, num: usize) {
    debug_assert!(num < 256);
    let word = self.wordof64_mut(num);
    *word &= !Self::maskof(num);
  }
  #[inline]
  const fn mark_free128(&mut self, num: usize) {
    debug_assert!(num < 128);
    let word = self.wordof128_mut(num);
    *word &= !Self::maskof(num);
  }
  #[inline]
  const fn mark_used64(&mut self, num: usize) {
    debug_assert!(num < 256);
    let word = self.wordof64_mut(num);
    *word |= Self::maskof(num);
  }
}

impl LowMemoryAllocator {
  #[inline]
  const fn buddyof64(num: usize) -> usize {
    debug_assert!(num < 256);
    num ^ 1
  }
  #[inline]
  const fn parentof64(num: usize) -> usize {
    debug_assert!(num < 256);
    num >> 1
  }
  #[inline]
  const fn isfree64(&self, num: usize) -> bool {
    (self.wordof64(num) >> Self::shiftof(num)) & 1 == 0
  }
  #[inline]
  const fn wordof64(&self, num: usize) -> usize {
    debug_assert!(num < 256);
    self.frames64[num / usize::BITS as usize]
  }
  #[inline]
  const fn wordof64_mut(&mut self, num: usize) -> &mut usize {
    debug_assert!(num < 256);
    &mut self.frames64[num / usize::BITS as usize]
  }
  #[inline]
  const fn wordof128_mut(&mut self, num: usize) -> &mut usize {
    debug_assert!(num < 128);
    &mut self.frames128[num / usize::BITS as usize]
  }
  #[inline]
  const fn shiftof(num: usize) -> usize {
    num % usize::BITS as usize
  }
  #[inline]
  const fn maskof(num: usize) -> usize {
    1 << Self::shiftof(num)
  }
  #[inline]
  const fn frame_num(word_index: usize, bitindex: usize) -> usize {
    word_index * usize::BITS as usize + bitindex
  }
  #[inline]
  const fn frame<S: FrameSize>(num: usize) -> Frame<S> {
    PhysAddr::new_truncate(num << S::SHIFT).frame()
  }
}
