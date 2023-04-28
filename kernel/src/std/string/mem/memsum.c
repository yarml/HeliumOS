#include <stddef.h>
#include <stdint.h>

int memsum(void const *block, size_t size)
{
  int sum = 0;
  for(; size != 0; --size, ++block)
    sum += *(int8_t*)block;
  return sum;
}