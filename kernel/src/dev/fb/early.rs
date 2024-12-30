use super::{CursorPosition, FbBuiltinFont};
use crate::{bootboot::BootbootHeader, dev::fb::FbFont};
use core::fmt::Write;

/// Early framebuffer always queries bootboot info on each write request for framebuffer location & dimensions,
/// does not use any buffering, writes directly to framebuffer, wraps around when screen is full
/// Supposed to only be used before heap allocation is possible.
pub struct EarlyFramebuffer {
  font: FbBuiltinFont,
  position: CursorPosition,
}

impl EarlyFramebuffer {
  pub const fn new() -> Self {
    Self {
      font: FbBuiltinFont::new(),
      position: CursorPosition::new(),
    }
  }
}

impl Write for EarlyFramebuffer {
  fn write_str(&mut self, s: &str) -> core::fmt::Result {
    let bootboot = BootbootHeader::instance();
    let fbinfo = bootboot.framebuffer();
    let fontdim = self.font.dimensions();
    let lines = fbinfo.height() / fontdim.height();
    let cols = fbinfo.width() / fontdim.width();
    let scanline = fbinfo.scanline();
    let mut buffer = fbinfo.acquire_buffer();
    for c in s.chars() {
      // Clear line if it just started
      if self.position.x == 0 {
        let start = self.position.y * fontdim.height() * scanline;
        let end = start + scanline * fontdim.height();
        buffer[start..end].fill(0);
      }
      match c {
        '\n' => {
          self.position.x = 0;
          self.position.y += 1;
        }
        '\r' => {}
        c => {
          let glyph = self.font.glyph(c);
          for (line, col) in (0..glyph.lines())
            .flat_map(|line| (0..glyph.cols()).map(move |col| (line, col)))
            .filter(|(line, col)| glyph.bitval(*col, *line))
          {
            let locy = self.position.y * fontdim.height() + line;
            let locx = self.position.x * fontdim.width() + 8 - col;
            let index = locy * fbinfo.scanline() + locx * 4;
            buffer[index..index + 4].fill(0xFF);
          }
          self.position.x += 1;
          if self.position.x > cols {
            self.position.x = 0;
            self.position.y += 1;
          }
        }
      }
      if self.position.y >= lines {
        self.position.y = 0;
      }
    }

    Ok(())
  }
}
