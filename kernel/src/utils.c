#include <ctype.h>
#include <stdio.h>
#include <utils.h>

char g_units_sign[UNITS_COUNT] = {'B', 'K', 'M', 'G', 'T', 'P', 'E'};

#define HEXDUMP_BPL (32)

void hexdump(void const *mem, size_t size) {
  size_t               linec = ALIGN_UP(size, HEXDUMP_BPL) / HEXDUMP_BPL;

  unsigned char const *cmem  = mem;

  for (size_t line = 0; line < linec; ++line) {
    printd("%p(+%05lu): ", mem + line * HEXDUMP_BPL, line * HEXDUMP_BPL);
    for (size_t i = 0; i < HEXDUMP_BPL; ++i) {
      size_t idx = line * HEXDUMP_BPL + i;
      if (idx < size)
        printd("%02x ", cmem[idx]);
      else
        printd("   ");
    }
    printd("\t");
    for (size_t i = 0; i < HEXDUMP_BPL; ++i) {
      size_t idx = line * HEXDUMP_BPL + i;
      if (idx < size && isprint(cmem[idx]))
        printd("%c", cmem[idx]);
      else
        printd(".");
    }
    printd("\n");
  }
}
