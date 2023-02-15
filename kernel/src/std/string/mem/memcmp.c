#include <stddef.h>

int memcmp (void const *b1, void const *b2, size_t size)
{
  // TODO: I am too lazy to think about this now, but I am sur there
  // exist some REP instruction to do this faster
  while(*(unsigned char*)b1 == *(unsigned char*)b2 && size != 0)
  {
    --size;
    ++b1;
    ++b2;
  }
  return *(unsigned char*)(b2 - 1) - *(unsigned char*)(b1 - 1);
}