#include <stdio.h>

static FILE *s_stdout = 0;
static FILE *s_stderr = 0;

void __init_stdio()
{
  s_stdout = fopen("debug://stdout", "a");
  s_stderr = fopen("debug://stderr", "a");
}

FILE *__get_stdout()
{
  return s_stdout;
}

FILE *__get_stderr()
{
  return s_stderr;
}

