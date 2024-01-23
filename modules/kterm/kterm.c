#include <boot_info.h>
#include <ctype.h>
#include <error.h>
#include <fs.h>
#include <psf.h>
#include <stdbool.h>
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

static term_state tstate;
static fsnode    *ctl;

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

typedef struct TRIPLET triplet;
struct TRIPLET {
  unsigned int a, b, c;
};
static triplet read_triplet(char const *b) {
  triplet t = {0, 0, 0};
  while (*b && isspace(*b)) {
    ++b;
  }

  t.a = stou(b, &b, 10);

  while (*b && isspace(*b)) {
    ++b;
  }
  t.b = stou(b, &b, 10);

  while (*b && isspace(*b)) {
    ++b;
  }
  t.c = stou(b, &b, 10);
  return t;
}

static void dfb_flush() {
  size_t *lowoff  = &tstate.fbinfo.dfb_lowest_off;
  size_t *highoff = &tstate.fbinfo.dfb_highest_off;
  if (tstate.fbinfo.use_dfb && *highoff != 0 && *lowoff != UINT64_MAX) {
    memcpy(
        tstate.fbinfo.fb + *lowoff,
        tstate.fbinfo.double_fb + *lowoff,
        *highoff - *lowoff
    );
    *lowoff  = UINT64_MAX;
    *highoff = 0;
  }
}

static size_t term_execute(char const *cmd, size_t size) {
  while (isspace(*cmd)) {
    ++cmd;
  }

  char label[size];
  memset(label, 0, size);

  size_t label_idx = 0;
  while (*cmd && !isspace(*cmd)) {
    label[label_idx] = *cmd;
    ++label_idx;
    ++cmd;
  }
  if (!*cmd) {
    --cmd;
    --label_idx;
  }
  label[label_idx] = 0;

  if (!strcmp(label, "fg")) {
    triplet t = read_triplet(cmd);
    tstate.fg = getcolor(tstate.fbinfo.format, t.a, t.b, t.c, 0);
  } else if (!strcmp(label, "bg")) {
    triplet t = read_triplet(cmd);
    tstate.bg = getcolor(tstate.fbinfo.format, t.a, t.b, t.c, 0);
  } else if (!strcmp(label, "flush")) {
    dfb_flush();
  } else if (!strcmp(label, "dfb_on")) {
    memcpy(tstate.fbinfo.double_fb, tstate.fbinfo.fb, tstate.fbinfo.fb_len);
    tstate.fbinfo.dfb_highest_off = 0;
    tstate.fbinfo.dfb_lowest_off  = UINT64_MAX;
    tstate.fbinfo.use_dfb         = true;
  } else if (!strcmp(label, "dfb_off")) {
    dfb_flush();
    tstate.fbinfo.use_dfb = false;
  } else {
    size_t len = strlen(label);
    printd("Unknown command: '%s' len: %ld\n", label, len);
    return 0;
  }

  return size;
}

static size_t term_file_append(fsnode *file, char const *buf, size_t size) {
  if (!strcmp(file->name, "ctl")) {
    return term_execute(buf, size);
  }
  if (strcmp(file->name, "stdout") && strcmp(file->name, "stderr")) {
    return 0;
  }
  uint8_t *target = tstate.fbinfo.double_fb && tstate.fbinfo.use_dfb
                      ? tstate.fbinfo.double_fb
                      : tstate.fbinfo.fb;

  for (size_t i = 0; i < size; ++i) {
    // If cursor is out of bound, scroll
    if (tstate.cursor >= tstate.cw * tstate.ch) {
      memmove(
          target,
          target + tstate.fontinfo.pixh * tstate.fbinfo.scanline,
          (tstate.ch - 1) * tstate.fontinfo.pixh * tstate.fbinfo.scanline
      );
      memset(
          target +
              (tstate.ch - 1) * tstate.fontinfo.pixh * tstate.fbinfo.scanline,
          0,
          tstate.fontinfo.pixh * tstate.fbinfo.scanline
      );
      tstate.cursor = (tstate.ch - 1) * tstate.cw;
      if (tstate.fbinfo.use_dfb) {
        // All screen should be refreshed now
        tstate.fbinfo.dfb_lowest_off  = 0;
        tstate.fbinfo.dfb_highest_off = tstate.fbinfo.fb_len;
      }
    }

    char cc = buf[i];
    // Handle special characters
    if (!cc || cc == '\a' || cc == '\b') {
      continue;
    }
    if (cc == '\n') {
      tstate.cursor =
          GALIGN_DN(tstate.cursor, tstate.cw) + tstate.cw + tstate.origin;
      continue;
    }
    if (cc == '\r') {
      tstate.cursor = GALIGN_DN(tstate.cursor, tstate.cw) + tstate.origin;
      continue;
    }
    if (cc == '\t') {
      tstate.cursor += tstate.tabsize;
      continue;
    }
    // In Helium terminals, \f and \v have special meanings, different from
    // other systems
    // \v: makes the current x position the x position \n and \r will
    //     return to
    // \f: makes 0 the x position \n and \r will return to
    // These will be useful when displaying trees of data
    if (cc == '\f') {
      tstate.origin = 0;
      continue;
    }
    if (cc == '\v') {
      tstate.origin = tstate.cursor % tstate.cw;
      continue;
    }
    if (cc == '\e') {
      size_t cmdend = 0;
      for (size_t j = i + 1; j < size; ++j) {
        if (buf[j] == '\e') {
          cmdend = j;
          break;
        }
      }
      if (!cmdend) {
        return size - i;
      }
      size_t cmdsize = cmdend - i - 1;
      term_file_append(ctl, buf + i + 1, cmdsize);
      i = cmdend;
      continue;
    }

    size_t glyphidx;
    if (cc < tstate.fontinfo.numglyphs) {
      glyphidx = cc;
    } else {
      glyphidx = 0;
    }

    uint8_t *glyph =
        tstate.fontinfo.glyphs + glyphidx * tstate.fontinfo.glyph_size;
    size_t bpl = ALIGN_UP(tstate.fontinfo.pixw, 8) / 8;
    for (size_t y = 0; y < tstate.fontinfo.pixh; ++y) {
      uint32_t linedata[tstate.fontinfo.pixw];
      memset(linedata, 0, sizeof(linedata));
      uint8_t *gline = glyph + y * bpl;
      for (size_t x = 0; x < tstate.fontinfo.pixw; ++x) {
        if (BIT(gline[x / 8], 8 - x % 8 - 1)) {
          linedata[x] = tstate.fg;
        } else {
          linedata[x] = tstate.bg;
        }
      }
      size_t cx = tstate.cursor % tstate.cw;
      size_t cy = tstate.cursor / tstate.cw;

      size_t px = cx * tstate.fontinfo.pixw;
      size_t py = cy * tstate.fontinfo.pixh + y;

      size_t wroff = px * 4 + py * tstate.fbinfo.scanline;
      memcpy(target + wroff, linedata, sizeof(linedata));
      if (wroff < tstate.fbinfo.dfb_lowest_off) {
        tstate.fbinfo.dfb_lowest_off = wroff;
      }
      if (wroff + 1 > tstate.fbinfo.dfb_highest_off) {
        tstate.fbinfo.dfb_highest_off = wroff + 1;
      }
    }
    ++tstate.cursor;
  }
  return size;
}

static int term_init_state(term_state *s, void *f) {
  memset(s, 0, sizeof(*s));

  uint8_t magic[] = {PSF2_MAGIC0, PSF2_MAGIC1, PSF2_MAGIC2, PSF2_MAGIC3};

  if (memcmp(f, magic, sizeof(magic))) {
    printd("PSF file does not have magic value.\n");
    return 1;
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

  return 0;
}

int module_init() {
  fsnode *font = fs_search("initrd://sys/font.psf");
  void   *f    = tarfs_direct_access(font);
  fs_close(font);
  int status = term_init_state(&tstate, f);
  if (status) {
    printd("Could not initialize terminal state.\n");
    return 1;
  }

  fsimpl impl;
  memset(&impl, 0, sizeof(impl));

  impl.fs_file_append = term_file_append;

  filesys *fs = fs_mount("term");
  if (!fs) {
    printd("Coud not mount 'term://'\n");
    return 1;
  }

  // Set fake dir capabilities to be able to build the immutable
  // tree, then remove these capabilities later
  fs->dir_cap = FSCAP_USED | FSCAP_DCREAT;

  fs->impl = impl;

  fsnode *fstdout = fs_mkfile(fs->root, "stdout");
  fsnode *fstderr = fs_mkfile(fs->root, "stderr");

  fsnode *fctl = fs_mkfile(fs->root, "ctl");

  fstdout->file.cap = FSCAP_USED | FSCAP_FAPPEND;
  fstderr->file.cap = FSCAP_USED | FSCAP_FAPPEND;
  fctl->file.cap    = FSCAP_USED | FSCAP_FAPPEND;

  ctl = fctl;

  fs->dir_cap = FSCAP_USED;

  return 0;
}