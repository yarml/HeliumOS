mod builtin_font;
mod early;
mod font;

pub use builtin_font::BuiltinFont as FbBuiltinFont;
pub use font::Font as FbFont;
pub use font::Glyph as FbGlyph;
pub use font::GlyphDimensions as FbGlyphDimensions;

use core::fmt::Write;
use early::EarlyFramebuffer;

pub struct Framebuffer {
  mode: FramebufferMode,
}

enum FramebufferMode {
  EarlyMode(EarlyFramebuffer),
}

struct CursorPosition {
  x: usize,
  y: usize,
}

impl Framebuffer {
  #[inline]
  pub const fn default() -> Self {
    Self {
      mode: FramebufferMode::EarlyMode(EarlyFramebuffer::new()),
    }
  }
}

impl CursorPosition {
  #[inline]
  pub const fn new() -> Self {
    Self { x: 0, y: 0 }
  }
}

impl Write for Framebuffer {
  fn write_str(&mut self, s: &str) -> core::fmt::Result {
    match &mut self.mode {
      FramebufferMode::EarlyMode(early_framebuffer) => {
        early_framebuffer.write_str(s)
      }
    }
  }
}
