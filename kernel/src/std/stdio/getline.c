#include <kterm.h>
#include <stdio.h>
#include <stdlib.h>

int getc() {
  // Don't judge
  char buf[2];
  kterm_fgets(buf, 2);
  return buf[0];
}

// Implementation copied from
// https://github.com/ivanrad/getline/blob/master/getline.c
size_t getdelim(char **restrict lineptr, size_t *restrict n, int delim) {
  char  *cur_pos, *new_lineptr;
  size_t new_lineptr_len;
  int    c;

  if (lineptr == NULL || n == NULL) {
    return -1;
  }

  if (*lineptr == NULL) {
    *n = 128; /* init len */
    if ((*lineptr = (char *)malloc(*n)) == NULL) {
      return -1;
    }
  }

  cur_pos = *lineptr;
  for (;;) {
    c = getc();

    if (c == EOF && cur_pos == *lineptr) {
      return -1;
    }

    if (c == EOF) {
      break;
    }

    if ((*lineptr + *n - cur_pos) < 2) {
      if (SIZE_MAX / 2 < *n) {
        return SIZE_MAX;
      }
      new_lineptr_len = *n * 2;

      if ((new_lineptr = (char *)realloc(*lineptr, new_lineptr_len)) == NULL) {
        return SIZE_MAX;
      }
      cur_pos  = new_lineptr + (cur_pos - *lineptr);
      *lineptr = new_lineptr;
      *n       = new_lineptr_len;
    }

    *cur_pos++ = (char)c;

    if (c == delim) {
      break;
    }
  }

  *cur_pos = '\0';
  return (size_t)(cur_pos - *lineptr);
}

size_t getline(char **restrict lineptr, size_t *restrict n) {
  return getdelim(lineptr, n, '\n');
}