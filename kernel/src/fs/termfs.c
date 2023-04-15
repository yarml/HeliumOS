#include <boot_info.h>
#include <string.h>
#include <stdio.h>
#include <psf.h>
#include <fs.h>

#include <fs/tar.h>

typedef struct TERM_STATE term_state;
struct TERM_STATE
{
  psf2_header *font;
};

static term_state tstate;

static size_t term_file_append(fsnode *file, char const *buf, size_t size)
{
  if (strcmp(file->name, "stdout") && strcmp(file->name, "stderr"))
    return 0;

  psf2_header *font = tstate.font;
  int x, y, kx = 0, line, mask, offs;
  int bpl = (font->width + 7) / 8;
  char const *s = buf;
  while (*s)
  {
    unsigned char *glyph = (unsigned char *)font + font->headersize +
                           (*s > 0 && *s < font->length ? *s : 0) * font->charsize;
    offs = (kx * (font->width + 1) * 4);
    for (y = 0; y < font->height; y++)
    {
      line = offs;
      mask = 1 << (font->width - 1);
      for (x = 0; x < font->width; x++)
      {
        *((uint32_t *)((uint64_t)&fb + line)) = ((int)*glyph) & (mask) ? 0xFFFFFF : 0;
        mask >>= 1;
        line += 4;
      }
      *((uint32_t *)((uint64_t)&fb + line)) = 0;
      glyph += bpl;
      offs += bootboot.fb_scanline;
    }
    s++;
    kx++;
  }
  return size;
}

static int term_init_state(term_state *s, void *f)
{
  memset(&tstate, 0, sizeof(tstate));

  uint8_t magic[] = {PSF2_MAGIC0, PSF2_MAGIC1, PSF2_MAGIC2, PSF2_MAGIC3};

  if (memcmp(f, magic, sizeof(magic)))
  {
    printd("PSF file does not have magic value.\n");
    return 1;
  }

  psf2_header *pf = f;

  printd("Width: %u\nHeight: %u\n", pf->width, pf->height);

  s->font = f;

  return 0;
}

void termfs_init()
{
  fsnode *font = fs_search("initrd://sys/font.psf");
  void *f = tarfs_direct_access(font);
  fs_close(font);

  if (term_init_state(&tstate, f))
  {
    printd("Could not initialize terminal state.\n");
    return;
  }

  fsimpl impl;
  memset(&impl, 0, sizeof(impl));

  impl.fs_file_append = term_file_append;

  filesys *fs = fs_mount("term");
  if (!fs)
  {
    printd("Coud not mount 'term://'\n");
    return;
  }

  // Set fake dir capabilities to be able to build the immutable
  // tree, then remove these capabilities later
  fs->dir_cap = FSCAP_USED | FSCAP_DCREAT;

  fs->impl = impl;

  fsnode *fstdout = fs_mkfile(fs->root, "stdout");
  fsnode *fstderr = fs_mkfile(fs->root, "stderr");

  fstdout->file.cap = FSCAP_USED | FSCAP_FAPPEND;
  fstderr->file.cap = FSCAP_USED | FSCAP_FAPPEND;

  fs->dir_cap = FSCAP_USED;
}