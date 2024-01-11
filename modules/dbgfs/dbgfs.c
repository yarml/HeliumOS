#include <debug.h>
#include <fs.h>
#include <stdio.h>
#include <string.h>

static size_t debug_file_append(fsnode *file, char const *buf, size_t size) {
  char lbuf[size + 1];
  memcpy(lbuf, buf, size);
  lbuf[size] = 0;
  return dbg_write_string(lbuf);
}

int module_init() {
  fsimpl impl;
  memset(&impl, 0, sizeof(impl));

  impl.fs_file_append = debug_file_append;

  filesys *fs = fs_mount("dbg");
  if (!fs) {
    printd("Could not mount 'dbg://'\n");
    return 1;
  }

  // Set fake dir capabilities to be able to build the immutable
  // tree, then remove these capabilities later
  fs->dir_cap = FSCAP_USED | FSCAP_DCREAT;

  fs->impl = impl;

  // Make them files
  fsnode *fostream = fs_mkfile(fs->root, "ostream");

  fostream->file.cap = FSCAP_USED | FSCAP_FAPPEND;

  fs->dir_cap = FSCAP_USED;

  return 0;
}
