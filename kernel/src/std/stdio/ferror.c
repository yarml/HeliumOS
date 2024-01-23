#include <stdio.h>

void clearerr(FILE *stream) {
  stream->err = 0;
}
int feof(FILE *stream) {
  return stream->eof;
}
int ferror(FILE *stream) {
  return stream->err;
}