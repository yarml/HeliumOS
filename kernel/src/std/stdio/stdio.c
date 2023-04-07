#include <stdio.h>

static FILE *s_stdout = 0;
static FILE *s_stderr = 0;

void __init_stdio()
{
  s_stdout = fopen("debug://stdout", "a");
  printf("stdout redirected to debug://stdout\n");
  s_stderr = fopen("debug://stderr", "a");
  printf("stderr redirected to debug://stderr\n");
}

FILE *__get_stdout()
{
  return s_stdout;
}

FILE *__get_stderr()
{
  return s_stderr;
}

