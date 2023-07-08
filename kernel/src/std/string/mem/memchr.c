#include <stddef.h>

#include <asm/scas.h>

void *memchr(void const *block, int c, size_t size) {
  size_t idx = size - as_scasb((uint64_t)block, c, size) - 1;
  return (((unsigned char *)block)[idx] == c) ? (void *)block + idx : 0;
}

void *memnchr(void const *block, int c, size_t size) {
  register size_t idx = size - as_nscasb((uint64_t)block, c, size) - 1;
  if (idx != size - 1) {
    return (void *)block + idx;
  }
  return (((unsigned char *)block)[idx] != c) ? (void *)block + idx : 0;
}
