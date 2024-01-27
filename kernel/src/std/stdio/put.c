#include <debug.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "internal_stdio.h"

int putchar(int c) {
  return dbg_write_chr(c);
}

int puts(char const *s) {
  return dbg_write_string(s);
}
