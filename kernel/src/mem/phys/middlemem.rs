//! # Middle Memory
//! Any usable memory in the range [16Mib 4Gib).
//! For DMA devices capable of it, and general purpose applications
//! when high memory cannot fullfill a request
//!
//! # Supported Requests
//! - Class 0: 4Kib on 4Kib boundary
//! - Class 0<N<9: Unavailable
//! - Class 9: 2Mib on 2Mib boundary
//!
//! # Kernel Space
//! Like low memory, this entire memory space can be found in kernel space
//! with constant offset using PhysAddr::to_virt()

use {
  super::frame::{size::Frame4KiB, Frame},
  crate::collections::AtomicU64Array,
  core::{
    mem,
    sync::atomic::{AtomicU64, Ordering},
  },
};

const BPW: usize = mem::size_of::<Word>();
const BTPW: usize = BPW * 8;
const WORDCOUNT: usize = (4 * 1024 * 1024 * 1024) / 4096 / BPW;

type Word = u64;
type AtomicWord = AtomicU64;
type AtomicArray = AtomicU64Array<WORDCOUNT>;

pub static MIDDLEMEM_ALLOCATOR: MiddleMemoryAllocator =
  MiddleMemoryAllocator::new();

// TODO: Maybe implement a DELAY-2 buddy. For now a bitmap
pub struct MiddleMemoryAllocator {
  bitmap: AtomicArray,
}

impl MiddleMemoryAllocator {
  pub const fn new() -> Self {
    Self {
      bitmap: AtomicArray::new([Word::MAX; WORDCOUNT]),
    }
  }
}

impl MiddleMemoryAllocator {
  pub fn alloc4(&self) -> Option<Frame<Frame4KiB>> {
    let iter = self
      .bitmap
      .iter()
      .enumerate()
      .filter(|(_, word)| *word != Word::MAX);
    for (index, _) in iter {
      if let Ok(word) = self.bitmap.fetch_update(
        index,
        Ordering::Relaxed,
        Ordering::Relaxed,
        |word| {
          if word == Word::MAX {
            return None;
          }
          let bitindex = word.trailing_ones();
          Some(word | (1 << bitindex))
        },
      ) {
        let bitindex = word.trailing_ones() as usize;
        let global_index = index * BTPW + bitindex;
        return Some(Frame::from_number(global_index));
      }
    }
    None
  }
  pub fn free4(&self, frame: Frame<Frame4KiB>) {
    let num = frame.number();
    let bitindex = num % BTPW;
    let windex = num / BTPW;
    self
      .bitmap
      .fetch_and(windex, !(1 << bitindex), Ordering::Relaxed);
  }
  pub fn free4_many(&self, start: Frame<Frame4KiB>, count: usize) {
    let start = start.number();
    let adjacent = start + count;
    let end = adjacent - 1;

    let start_word = start / BTPW;
    let end_word = end / BTPW;

    let start_spill = BTPW - start % BTPW;
    let end_spill = adjacent % BTPW;
    let start_mask = if start_spill == 0 {
      0
    } else {
      Word::MAX << (BTPW - start_spill)
    };
    let end_mask = if end_spill == 0 {
      0
    } else {
      Word::MAX >> (BTPW - end_spill)
    };

    if start_word == end_word {
      self.bitmap.fetch_and(
        start_word,
        start_mask & end_mask,
        Ordering::Relaxed,
      );
    } else {
      let middle_count = (count - start_spill - end_spill) / BTPW;
      self
        .bitmap
        .fetch_and(start_word, start_mask, Ordering::Relaxed);
      self.bitmap.fetch_and(end_word, end_mask, Ordering::Relaxed);
      for i in 1..=middle_count {
        self.bitmap.store(i + start_word, 0, Ordering::Relaxed);
      }
    }
  }
}
