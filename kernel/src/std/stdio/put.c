#include <string.h>
#include <stdio.h>
#include <debug.h>

int fputc(int c, FILE *stream)
{
  if(!stream)
    return dbg_write_chr(c);
  return c;
}

int putchar(int c)
{
  return fputc(c, stdout);
}

int fputs(char const *s, FILE *stream)
{
  if(!stream)
    return dbg_write_string(s);
  return 0; // TODO: Not implemented
}

int puts(char const *s)
{
  return fputs(s, stdout);
}
