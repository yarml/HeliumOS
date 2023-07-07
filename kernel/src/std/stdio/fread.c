#include <errno.h>
#include <fs.h>
#include <stdio.h>

#include "internal_stdio.h"

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  if (stream->mode & MODE_R) {
    errno = 0;
    size_t read = fs_read(stream->fnode, stream->cur, ptr, size * nmemb);

    if (!read) {
      if (errno)
        stream->err = 1;
      else
        stream->eof = 1;
      return 0;
    }

    stream->cur += read;
    return read / size;
  }

  stream->err = 1;
  return 0;
}

size_t fpull(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  if (stream->mode & MODE_P) {
    errno = 0;
    size_t read = fs_pull(stream->fnode, ptr, size * nmemb);

    if (!read) {
      if (errno)
        stream->err = 1;
      else
        stream->eof = 1;
      return 0;
    }
    return read / size;
  }

  stream->err = 1;
  return 0;
}
