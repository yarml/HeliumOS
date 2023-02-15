#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <utils.h>

#include <debug.h>

FILE *stdout;
FILE *stderr;

static FILE __stdout;
static FILE __stderr;

void __init_stdio()
{
  stdout = &__stdout;
  stderr = &__stderr;

  __stdout = dbg_output_file();
  __stderr = dbg_output_file();
}
