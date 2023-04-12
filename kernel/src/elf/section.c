#include <string.h>
#include <elf.h>

size_t elf_sect_len(void *kmodf)
{
  return ((elf64_header *) kmodf)->sht_len;
}

elf64_sect_header *elf_sect_hdr(void *kmodf, size_t shidx)
{
  elf64_header *hdr = kmodf;
  return kmodf + hdr->shoff + shidx * hdr->shent_size;
}

void *elf_sect_content(void *kmodf, size_t shidx)
{
  elf64_sect_header *sect = elf_sect_hdr(kmodf, shidx);
  return kmodf + sect->offset;
}

size_t elf_sect_ent_len(void *kmodf, size_t shidx)
{
  elf64_sect_header *sect = elf_sect_hdr(kmodf, shidx);
  return sect->size / sect->ent_size;
}

void *elf_sect_ent(void *kmodf, size_t shidx, size_t entidx)
{
  elf64_sect_header *sect = elf_sect_hdr(kmodf, shidx);
  return kmodf + sect->offset + entidx * sect->ent_size;
}

char *elf_shstrtab(void *kmodf)
{
  elf64_header *eh = kmodf;
  return elf_sect_content(kmodf, eh->shstridx);
}

char *elf_strtab(void *kmodf)
{
  char *shstrtab = elf_shstrtab(kmodf);
  size_t sl = elf_sect_len(kmodf);

  for(size_t i = 0; i < sl; ++i)
  {
    elf64_sect_header *sh = elf_sect_hdr(kmodf, i);
    if(sh->type != SHT_STRTAB)
      continue;
    if(!strcmp(shstrtab + sh->name, ".strtab"))
      return elf_sect_content(kmodf, i);
  }
  return 0;
}
