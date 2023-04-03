#include <ctype.h>
#include <utils.h>
#include <stdio.h>

char g_units_sign[UNITS_COUNT] = {'B', 'K', 'M', 'G', 'T', 'P', 'E'};

#define HEXDUMP_BPL (32)

void hexdump(void *mem, size_t size)
{
  size_t linec = ALIGN_UP(size, HEXDUMP_BPL) / HEXDUMP_BPL;

  unsigned char *cmem = mem;

  for(size_t line = 0; line < linec; ++line)
  {
    printf("%p(+%05lu): ", mem + line * HEXDUMP_BPL, line*HEXDUMP_BPL);
    for(size_t i = 0; i < HEXDUMP_BPL; ++i)
    {
      size_t idx = line * HEXDUMP_BPL + i;
      if(idx < size)
        printf("%02x ", cmem[idx]);
      else
        printf("   ");
    }
    printf("\t");
    for(size_t i = 0; i < HEXDUMP_BPL; ++i)
    {
      size_t idx = line * HEXDUMP_BPL + i;
      if(idx < size && isprint(cmem[idx]))
        printf("%c", cmem[idx]);
      else
        printf(".");
    }
    printf("\n");
  }
}
