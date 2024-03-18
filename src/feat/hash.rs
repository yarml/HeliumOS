use core::{hash::Hasher, num::Wrapping};

pub struct DefaultHasher {
  state: Wrapping<u64>,
}

impl DefaultHasher {
  pub const fn new() -> Self {
    Self {
      state: Wrapping(525201411107845655),
    }
  }
}

impl Hasher for DefaultHasher {
  fn finish(&self) -> u64 {
    self.state.0
  }

  fn write(&mut self, bytes: &[u8]) {
    // from https://stackoverflow.com/a/57960443/21296545
    // I have no clue how this works
    for b in bytes {
      self.state ^= *b as u64;
      self.state *= 0x5bd1e9955bd1e995;
      self.state ^= self.state >> 47;
    }
  }
}
