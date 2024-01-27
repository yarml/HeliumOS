#include <boot_info.h>
#include <fs.h>
#include <psf.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>

#include <fs/tar.h>

typedef struct FB_SPEC fb_spec;
struct FB_SPEC {
  uint8_t *fb;
  uint8_t *double_fb;
  size_t dfb_lowest_off;  // Lowest offset from fb that was written to after the
                          // last flush, inclusive
  size_t dfb_highest_off;  // Highest offset from fb that was written to after
                           // the last flush, exclusive
  bool use_dfb;

  size_t fb_len;

  size_t pixw;
  size_t pixh;

  size_t scanline;

  uint8_t format;
};

typedef struct FONT_SPEC font_spec;
struct FONT_SPEC {
  uint8_t *glyphs;
  size_t   numglyphs;

  size_t glyph_size;

  size_t pixw;
  size_t pixh;
};

typedef struct TERM_STATE term_state;
struct TERM_STATE {
  fb_spec   fbinfo;
  font_spec fontinfo;

  // Cursor in character position
  size_t cursor;

  // Screen size in charatcer units
  size_t cw;
  size_t ch;

  // Tab size in character unit
  size_t tabsize;

  size_t origin;  // x Offset that \n and \r return to

  uint32_t fg;
  uint32_t bg;
};

static term_state state;

static uint32_t getcolor(
    uint8_t format, uint8_t r, uint8_t g, uint8_t b, uint8_t a
) {
  switch (format) {
    case FB_ARGB:
      return (uint32_t)a << 24 | (uint32_t)r << 16 | (uint32_t)g << 8 | b;
    case FB_RGBA:
      return (uint32_t)r << 24 | (uint32_t)g << 16 | (uint32_t)b << 8 | a;
    case FB_ABGR:
      return (uint32_t)a << 24 | (uint32_t)b << 16 | (uint32_t)g << 8 | r;
    case FB_BGRA:
      return (uint32_t)b << 24 | (uint32_t)g << 16 | (uint32_t)r << 8 | a;
    default:
      printd("Unknown color format: %x\n", format);
      error_inv_state("Invalid color format\n");
  }
}

void kterm_init() {
  fsnode *font = fs_search("initrd://sys/font.psf");
  void   *f    = tarfs_direct_access(font);
  fs_close(font);

  term_state *s = &state;

  memset(s, 0, sizeof(*s));

  uint8_t magic[] = {PSF2_MAGIC0, PSF2_MAGIC1, PSF2_MAGIC2, PSF2_MAGIC3};

  if (memcmp(f, magic, sizeof(magic))) {
    printd("PSF file does not have magic value.\n");
    return;
  }

  psf2_header *pf = f;

  size_t fb_len = bootboot.fb_height * bootboot.fb_scanline;

  uint8_t *double_fb = malloc(fb_len);

  s->fbinfo.fb              = &fb;
  s->fbinfo.double_fb       = double_fb;
  s->fbinfo.fb_len          = fb_len;
  s->fbinfo.use_dfb         = false;
  s->fbinfo.dfb_lowest_off  = UINT64_MAX;
  s->fbinfo.dfb_highest_off = 0;

  s->fbinfo.pixw = bootboot.fb_width;
  s->fbinfo.pixh = bootboot.fb_height;

  s->fbinfo.scanline = bootboot.fb_scanline;

  s->fbinfo.format = bootboot.fb_type;

  s->fontinfo.glyphs     = f + pf->headersize;
  s->fontinfo.numglyphs  = pf->length;
  s->fontinfo.glyph_size = pf->charsize;
  s->fontinfo.pixw       = pf->width;
  s->fontinfo.pixh       = pf->height;

  s->cw = s->fbinfo.pixw / s->fontinfo.pixw;
  s->ch = s->fbinfo.pixh / s->fontinfo.pixh;

  s->tabsize = 2;

  s->fg = getcolor(s->fbinfo.format, 255, 255, 255, 0);
  s->bg = getcolor(s->fbinfo.format, 0, 0, 0, 0);
}

size_t kterm_print(char const *str) {
  size_t   size   = strlen(str);
  uint8_t *target = state.fbinfo.double_fb && state.fbinfo.use_dfb
                      ? state.fbinfo.double_fb
                      : state.fbinfo.fb;

  for (size_t i = 0; i < size; ++i) {
    // If cursor is out of bound, scroll
    if (state.cursor >= state.cw * state.ch) {
      memmove(
          target,
          target + state.fontinfo.pixh * state.fbinfo.scanline,
          (state.ch - 1) * state.fontinfo.pixh * state.fbinfo.scanline
      );
      memset(
          target + (state.ch - 1) * state.fontinfo.pixh * state.fbinfo.scanline,
          0,
          state.fontinfo.pixh * state.fbinfo.scanline
      );
      state.cursor = (state.ch - 1) * state.cw;
      if (state.fbinfo.use_dfb) {
        // All screen should be refreshed now
        state.fbinfo.dfb_lowest_off  = 0;
        state.fbinfo.dfb_highest_off = state.fbinfo.fb_len;
      }
    }

    char cc = str[i];

    // Handle special characters, only fall through this switch statement if
    // character is printable
    switch (cc) {
      // Unsupported, for now
      case '\a':
      case '\b':
        continue;
      case '\n':
        state.cursor =
            GALIGN_DN(state.cursor, state.cw) + state.cw + state.origin;
        continue;
      case '\r':
        state.cursor = GALIGN_DN(state.cursor, state.cw) + state.origin;
        continue;
      case '\t':
        state.cursor += state.tabsize;
        continue;
      // In Helium terminals, \f and \v have special meanings, different from
      // other systems
      // \v: makes the current x position the x position \n and \r will
      //     return to
      // \f: makes 0 the x position \n and \r will return to
      // These will be useful when displaying trees of data
      case '\f':
        state.origin = 0;
        continue;
      case '\v':
        state.origin = state.cursor % state.cw;
        continue;
    }
    size_t glyphidx;
    if (cc < state.fontinfo.numglyphs) {
      glyphidx = cc;
    } else {
      glyphidx = 0;
    }

    uint8_t *glyph =
        state.fontinfo.glyphs + glyphidx * state.fontinfo.glyph_size;
    size_t bpl = ALIGN_UP(state.fontinfo.pixw, 8) / 8;
    for (size_t y = 0; y < state.fontinfo.pixh; ++y) {
      uint32_t linedata[state.fontinfo.pixw];
      memset(linedata, 0, sizeof(linedata));
      uint8_t *gline = glyph + y * bpl;
      for (size_t x = 0; x < state.fontinfo.pixw; ++x) {
        if (BIT(gline[x / 8], 8 - x % 8 - 1)) {
          linedata[x] = state.fg;
        } else {
          linedata[x] = state.bg;
        }
      }
      size_t cx = state.cursor % state.cw;
      size_t cy = state.cursor / state.cw;

      size_t px = cx * state.fontinfo.pixw;
      size_t py = cy * state.fontinfo.pixh + y;

      size_t wroff = px * 4 + py * state.fbinfo.scanline;
      memcpy(target + wroff, linedata, sizeof(linedata));
      if (wroff < state.fbinfo.dfb_lowest_off) {
        state.fbinfo.dfb_lowest_off = wroff;
      }
      if (wroff + 1 > state.fbinfo.dfb_highest_off) {
        state.fbinfo.dfb_highest_off = wroff + 1;
      }
    }
    ++state.cursor;
  }
  return size;
}