#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fs.h>

#include "internal_stdio.h"

FILE *fopen(char *path, char *mode)
{
  // Parse file mode
  size_t mode_len = strlen(mode);
  if(!mode_len || mode_len > 2)
  {
    errno = EINVAL;
    return 0;
  }

  int imode = 0;
  switch(mode[0])
  {
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

  if(mode_len > 1)
  {
    switch(mode[1])
    {
      case '+':
        if(imode & MODE_R)
          imode |= MODE_W;
        else
          imode |= MODE_R;
        break;
      default:
        errno = EINVAL;
        return 0;
    }
  }

  if(!fs_valid_path(path))
  {
    errno = EINVAL;
    return 0;
  }


  fsnode *fnode = fs_search(path);
  if(!fnode)
  {
    // This means file does not exist
    // If mode is append or write, we create file
    // if readonly, we return with error

    if(!(imode & MODE_W) && !(imode & MODE_A))
      return 0; // errno set by fs_search

    fsnode *fdir = fs_dirof(path);

    if(!fdir)
    {
      errno = ENOENT;
      return 0;
    }
    char fname[FSNODE_NAMELEN];
    fs_basename(path, fname);
    fnode = fs_mkfile(fdir, fname);
    fs_close(fdir);
    if(!fnode)
      return 0; // errno set by fs_mkfile
  }

  if(fnode->type == FSNODE_DIR)
  {
    fs_close(fnode);
    errno = EISDIR;
    return 0;
  }

  if((imode & MODE_R) && !(fnode->file.cap & FSCAP_FREAD))
  {
    fs_close(fnode);
    errno = EOPNOTSUPP;
    return 0;
  }

  if((imode & MODE_W) && !(fnode->file.cap & FSCAP_FWRITE))
  {
    fs_close(fnode);
    errno = EOPNOTSUPP;
    return 0;
  }

  if((imode & MODE_A) && !(fnode->file.cap & FSCAP_FAPPEND))
  {
    fs_close(fnode);
    errno = EOPNOTSUPP;
    return 0;
  }

  FILE *file = calloc(1, sizeof(FILE));;
  if(!file) // errno set by calloc
    return 0;

  file->fnode = fnode;
  file->mode = imode;

  return file;
}

int fclose(FILE *stream)
{
  fs_close(stream->fnode);
  free(stream);
  return 0;
}