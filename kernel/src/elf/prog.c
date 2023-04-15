#include <elf.h>

size_t elf_proght_len(void *f)
{
  return ((elf64_header *) f)->pht_len;
}

elf64_prog_header *elf_prog_hdr(void *f, size_t phidx)
{
  elf64_header *eh = f;
  return f + eh->phoff + phidx * eh->phent_size;
}
