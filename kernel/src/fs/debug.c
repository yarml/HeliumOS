#include <string.h>
#include <stdio.h>
#include <debug.h>
#include <fs.h>

static size_t debug_file_append(fsnode *file, char *buf, size_t size)
{
  char lbuf[size+1];
  memcpy(lbuf, buf, size);
  lbuf[size] = 0;
  return dbg_write_string(lbuf);
}

void debug_initfs()
{
  fsimpl impl;
  memset(&impl, 0, sizeof(impl));

  impl.fs_file_append = debug_file_append;

  filesys *fs = fs_mount("debug");
  if(!fs)
  {
    printf("Could not mount 'debug://'\n");
    return;
  }

  // Set fake dir capabilities to be able to build the immutable
  // tree, then remove these capabilities later
  fs->dir_cap = FSCAP_USED | FSCAP_DCREAT;

  fs->impl = impl;

  // Make them files
  fsnode *fstdout = fs_mkfile(fs->root, "stdout");
  fs_mklink(fs->root, "stderr", fstdout);

  fstdout->file.cap = FSCAP_USED | FSCAP_FAPPEND;

  fs->dir_cap = FSCAP_USED;
}
