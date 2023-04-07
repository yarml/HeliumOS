#include <errno.h>
#include <stdio.h>

#include "internal_stdio.h"

size_t fwrite(
  void *ptr,
  size_t size,
  size_t nmemb,
  FILE *stream
) {
  if(stream->mode & MODE_W)
  {
    size_t written = fs_write(stream->fnode, stream->cur, ptr, size*nmemb);

    if(written < size*nmemb)
      stream->err = 1;

    stream->cur += written;
    return written/size;
  }

  stream->err = 1;
  return 0;
}

size_t fappend(
  void *ptr,
  size_t size,
  size_t nmemb,
  FILE *stream
) {
  if(stream->mode & MODE_W)
  {
    size_t written = fs_append(stream->fnode, ptr, size*nmemb);

    if(written < size*nmemb)
      stream->err = 1;
    return written/size;
  }

  stream->err = 1;
  return 0;
}

