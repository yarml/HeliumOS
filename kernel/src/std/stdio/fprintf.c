#include <stdarg.h>
#include <stdio.h>

int tpf(char const *template, ...)
{
  /* Implementation copied from printf() */
  va_list va;
  va_start(va, template);
  int ret = vprintf(template, va);
  va_end(va);
  return ret;
}


int printf(char const *template, ...)
{
  // TODO: If this function changes, tpf() should also be updated to be
  // similar
  va_list va;
  va_start(va, template);
  int ret = vprintf(template, va);
  va_end(va);
  return ret;
}

int vprintf(char const *template, va_list va)
{
  return vfprintf(stdout, template, va);
}


int fprintf(FILE *stream, char const *template, ...)
{
  va_list va;
  va_start(va, template);
  int ret = vfprintf(stream, template, va);
  va_end(va);
  return ret;
}

int vfprintf(FILE *stream, char const *template, va_list va)
{
  va_list va2;
  va_copy(va2, va);
  size_t len = vsnprintf(0, 0, template, va2);
  va_end(va2);

  char buf[len + 1];

  vsnprintf(buf, len + 1, template, va);
  int ret = stream->write_string(stream, buf);
  return ret ? ret : len;
}