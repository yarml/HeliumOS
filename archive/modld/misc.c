#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

FILE *fopen_or_exit(char const *path, char const *mode)
{
  FILE *f = fopen(path, mode);

  if(!f)
  {
    fprintf(stderr, "Could not open file '%s' with mode '%s'\n", path, mode);
    exit(1);
  }

  return f;
}

void *calloc_or_exit(size_t n, size_t size)
{
  void *p = calloc(n, size);
  if(!p)
  {
    perror("Could not allocate memory");
    exit(1);
  }
  return p;
}

uint64_t elf_hash(char const *name)
{
  uint64_t h = 0, g;
  uint8_t const *uname = (void *) name;
  while(*uname)
  {
    h = (h << 4) + *uname++;
    if((g = h & 0xf0000000))
      h ^= g >> 24;
    h &= ~g;
  }
  return h;
}
