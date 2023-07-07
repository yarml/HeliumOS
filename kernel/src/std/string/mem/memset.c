#include <stddef.h>
#include <stdint.h>

#include <asm/stos.h>

#include "../internal_string.h"

void *memset(void *block, int c, size_t size) {
  void *oblock = block;

  while ((uintptr_t)block % 8 && size) {
    *(uint8_t *)block++ = c;
    --size;
  }
  if (size >= MASS_OP_MIN_BYTES) {
    uint64_t c64 = c;
    c64          = c64 << 8 | c64;
    c64          = c64 << 16 | c64;
    c64          = c64 << 32 | c64;
    as_stosq((uint64_t)block, c64, size / 8);
    block += size / 8;
    size -= size / 8;
  }
  as_stosb((uint64_t)block, c, size);

  return oblock;
}