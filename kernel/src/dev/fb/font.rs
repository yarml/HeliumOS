use core::{
  iter::{Filter, FlatMap, Map},
  ops::Range,
};

pub struct GlyphDimensions {
  width: usize,
  height: usize,
}

pub struct Glyph<'a> {
  bitmap: &'a [u8],
  lines: usize,
}

pub trait Font {
  fn dimensions(&self) -> GlyphDimensions;
  fn glyph<'a>(&'a self, c: char) -> Glyph<'a>;
}

impl GlyphDimensions {
  #[inline]
  pub const fn new(width: usize, height: usize) -> Self {
    Self { width, height }
  }
}

impl GlyphDimensions {
  pub fn width(&self) -> usize {
    self.width
  }
  pub fn height(&self) -> usize {
    self.height
  }
}

impl<'a> Glyph<'a> {
  #[inline]
  pub const fn from(bitmap: &'a [u8], lines: usize) -> Self {
    Self { bitmap, lines }
  }
}

impl<'a> Glyph<'a> {
  #[inline]
  pub const fn bytesperline(&self) -> usize {
    self.bitmap.len() / self.lines
  }
  #[inline]
  pub const fn cols(&self) -> usize {
    self.bytesperline() * 8
  }
  #[inline]
  pub const fn lines(&self) -> usize {
    self.lines
  }
  #[inline]
  pub const fn bitmap(&self) -> &'a [u8] {
    self.bitmap
  }
  #[inline]
  pub fn bitval(&self, x: usize, y: usize) -> bool {
    let bpl = self.bytesperline();
    (self.bitmap[bpl * y..bpl * (y + 1)][x / 8] >> (x % 8)) & 1 == 1
  }
}
