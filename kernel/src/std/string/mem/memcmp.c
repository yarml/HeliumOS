#include <stddef.h>
#include <stdio.h>

int memcmp(void const *b1, void const *b2, size_t size) {
  if (b1 == b2) return 0;

  // TODO: I am too lazy to think about this now, but I am sur there
  // exist some REP instruction to do this faster
  while (--size && *(unsigned char *)b1 == *(unsigned char *)b2) {
    ++b1;
    ++b2;
  }
  return *(unsigned char *)(b2) - *(unsigned char *)(b1);
}