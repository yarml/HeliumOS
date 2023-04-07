#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <debug.h>

#include "internal_stdio.h"

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

  size_t s_len = strlen(s);
  if(stream->mode & MODE_W)
    return fwrite(s, sizeof(char), s_len+1, stream);
  else if(stream->mode & MODE_A)
    return fappend(s, sizeof(char), s_len+1, stream);
  else
  {
    errno = EOPNOTSUPP;
    return 0;
  }
}

int puts(char const *s)
{
  return fputs(s, stdout);
}
