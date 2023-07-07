#include <errno.h>
#include <fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal_stdio.h"

FILE *fopen(char *path, char *mode) {
  int imode = 0;

  // Parse file mode
  if (mode) {
    size_t mode_len = strlen(mode);

    for (size_t i = 0; i < mode_len; ++i) {
      switch (mode[i]) {
        case 'p':  // HeliumOS specific
          imode |= MODE_P;
          break;
        case 'r':
          imode |= MODE_R;
          break;
        case 'w':
          imode |= MODE_W;
          break;
        case 'a':
          imode |= MODE_A;
          break;
        default:
          errno = EINVAL;
          return 0;
      }
    }
  }

  if (!fs_valid_path(path)) {
    errno = EINVAL;
    return 0;
  }

  fsnode *fnode = fs_search(path);
  if (!fnode) {
    // This means file does not exist
    // If mode is append or write, we create file
    // if readonly, we return with error

    if (!(imode & MODE_W) && !(imode & MODE_A))
      return 0;  // errno set by fs_search

    fsnode *fdir = fs_dirof(path);

    if (!fdir) {
      errno = ENOENT;
      return 0;
    }
    char fname[FSNODE_NAMELEN];
    fs_basename(path, fname);
    fnode = fs_mkfile(fdir, fname);
    fs_close(fdir);
    if (!fnode) return 0;  // errno set by fs_mkfile
  }

  if (fnode->type == FSNODE_DIR) {
    fs_close(fnode);
    errno = EISDIR;
    return 0;
  }

  if ((imode & MODE_R) && !fs_check_fcap(fnode, FSCAP_FREAD)) {
    fs_close(fnode);
    errno = EOPNOTSUPP;
    return 0;
  }

  if ((imode & MODE_W) && !fs_check_fcap(fnode, FSCAP_FWRITE)) {
    fs_close(fnode);
    errno = EOPNOTSUPP;
    return 0;
  }

  if ((imode & MODE_A) && !fs_check_fcap(fnode, FSCAP_FAPPEND)) {
    fs_close(fnode);
    errno = EOPNOTSUPP;
    return 0;
  }

  if ((imode & MODE_P) && !fs_check_fcap(fnode, FSCAP_FPULL)) {
    fs_close(fnode);
    errno = EOPNOTSUPP;
    return 0;
  }

  FILE *file = calloc(1, sizeof(FILE));
  if (!file)  // errno set by calloc
    return 0;

  file->fnode = fnode;
  file->mode = imode;

  return file;
}

int fclose(FILE *stream) {
  fs_close(stream->fnode);
  free(stream);
  return 0;
}