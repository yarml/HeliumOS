#include <debug.h>
#include <errno.h>
#include <kterm.h>
#include <stdarg.h>
#include <stdio.h>

#include "internal_stdio.h"

int tpd(char const *template, ...) {
  /* Implementation copied from printd() */
  va_list va;
  va_start(va, template);
  int ret = vprintd(template, va);
  va_end(va);
  return ret;
}

int tpf(char const *template, ...) {
  /* Implementation copied from printf() */
  va_list va;
  va_start(va, template);
  int ret = vprintf(template, va);
  va_end(va);
  return ret;
}

int prtrace_begin(char const *fname, char const *args, ...) {
  va_list va;
  va_start(va, args);

  int r = 0;
#ifdef PRINT_FUNC_TRACE
  r += printd("begin %s(", fname);
  if (args) {
    r += vprintf(args, va);
  }
  r += printd(")\n");
#endif
  va_end(va);
  return r;
}

int prtrace_end(
    char const *fname, char const *status, char const *result, ...
) {
  va_list va;
  va_start(va, result);

  int r = 0;
#ifdef PRINT_FUNC_TRACE
  r += printd("end %s()", fname);
  if (status) {
    printd(" -> %s", status);
  }
  if (result) {
    printd("{");
    vprintf(result, va);
    printd("}");
  }
  printd("\n");
#endif
  va_end(va);
  return r;
}

int printd(char const *template, ...) {
  va_list va;
  va_start(va, template);
  int ret = vprintd(template, va);
  va_end(va);
  return ret;
}

int vprintd(char const *template, va_list va) {
  va_list va2;
  va_copy(va2, va);
  size_t len = vsnprintf(0, 0, template, va2);
  va_end(va2);
  char buf[len + 1];
  vsnprintf(buf, len + 1, template, va);
  return dbg_write_string(buf);
}

int printf(char const *template, ...) {
  va_list va;
  va_start(va, template);
  int ret = vprintf(template, va);
  va_end(va);
  return ret;
}
int vprintf(char const *template, va_list va) {
  va_list va2;
  va_copy(va2, va);
  size_t len = vsnprintf(0, 0, template, va2);
  va_end(va2);

  char buf[len + 1];

  vsnprintf(buf, len + 1, template, va);

  return kterm_print(buf);
}
