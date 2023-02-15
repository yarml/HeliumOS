#include <string.h>
#include <stdio.h>

int fputc(int c, FILE *stream)
{
    if(stream->write_chr(stream, c))
      return EOF;
    return c;
}

int putchar(int c)
{
  return fputc(c, stdout);
}

int fputs(char const *s, FILE *stream)
{
  return stream->write_string(stream, s) ? EOF : strlen(s);
}

int puts(char const *s)
{
  return fputs(s, stdout);
}
