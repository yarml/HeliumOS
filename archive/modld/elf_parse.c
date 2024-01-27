#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "modld.h"

static char const *whitelisted_sections[] = {
  ".text", ".rodata", ".data", ".bss",
  ".rela.text", ".rela.rodata", ".rela.data", ".rela.bss",
  ".symtab", ".strtab", ".shstrtab", ".comment"
};
#define WHITELISTED_SECTIONS_LEN (sizeof(whitelisted_sections) / sizeof(char *))
static uint64_t whiteliested_sections_hash[WHITELISTED_SECTIONS_LEN];

void elf_parser_init()
{
  for(size_t i = 0; i < WHITELISTED_SECTIONS_LEN; ++i)
  {
    uint64_t hash = elf_hash(whitelisted_sections[i]);
    whiteliested_sections_hash[i] = hash;
  }
}

Elf64_Ehdr *verify_ehdr(uint8_t *frel_data, size_t frel_size)
{
  if(frel_size < sizeof(Elf64_Ehdr))
  {
    fprintf(stderr, "File too small: %zu\n", frel_size);
    exit(1);
  }

  if(memcmp(frel_data, ELFMAG, 4))
  {
    fprintf(
      stderr,
      "File not in ELF format: %02x%02x%02x%02x\n",
      frel_data[EI_MAG0], frel_data[EI_MAG1],
      frel_data[EI_MAG2], frel_data[EI_MAG3]
    );
    exit(1);
  }

  if(frel_data[EI_CLASS] != ELFCLASS64)
  {
    fprintf(
      stderr,
      "ELF file not in 64-bit format: %#02x\n",
      frel_data[EI_CLASS]
    );
    exit(1);
  }
  if(frel_data[EI_DATA] != ELFDATA2LSB)
  {
    fprintf(
      stderr,
      "ELF file not in LSB format: %#02x\n",
      frel_data[EI_DATA]
    );
    exit(1);
  }
  if(frel_data[EI_VERSION] != EV_CURRENT)
  {
    fprintf(
      stderr,
      "ELF file with unrecognized version: %02x\n",
      frel_data[EI_VERSION]
    );
    exit(1);
  }
  if(frel_data[EI_OSABI] != ELFOSABI_NONE)
  {
    fprintf(
      stderr,
      "ELF file with unrecognized OS ABI: %02x\n",
      frel_data[EI_OSABI]
    );
    exit(1);
  }
  if(frel_data[EI_ABIVERSION] != 0)
  {
    fprintf(
      stderr,
      "ELF file with unrecognized ABI version: %#02x\n",
      frel_data[EI_ABIVERSION]
    );
    exit(1);
  }
  Elf64_Ehdr *eh = (void *) frel_data;
  if(eh->e_type != ET_REL)
  {
    fprintf(
      stderr,
      "ELF file not relocatable: %#04x\n",
      eh->e_type
    );
    exit(1);
  }
  if(eh->e_machine != EM_X86_64)
  {
    fprintf(
      stderr,
      "ELF file targeting unsupported machine: %#04x\n",
      eh->e_machine
    );
    exit(1);
  }
  if(eh->e_version != EV_CURRENT)
  {
    fprintf(
      stderr,
      "ELF file with unrecognized version: %#08x\n",
      eh->e_version
    );
    exit(1);
  }
  if(eh->e_flags)
  {
    fprintf(
      stderr,
      "ELF file with unrecognized flags: %#08x\n",
      eh->e_flags
    );
    exit(1);
  }
  return eh;
}

char const *getstrtab(Elf64_Ehdr *eh)
{
  for(size_t i = 0; i < eh->e_shnum; ++i)
  {
    Elf64_Shdr *sh = getsh(eh, i);
    if(!sh)
      continue;
    if(sh->sh_type != SHT_STRTAB)
      continue;
    char const *shname = get_shstr(eh, sh->sh_name);
    if(!strcmp(shname, ".strtab"))
      return get_shcontent(eh, sh);
  }
  fprintf(stderr, "Object file's string table not found.\n");
  exit(1);
}

Elf64_Shdr *getsymtab(Elf64_Ehdr *eh)
{
  for(size_t i = 0; i < eh->e_shnum; ++i)
  {
    Elf64_Shdr *sh = getsh(eh, i);
    if(!sh)
      continue;
    if(sh->sh_type != SHT_SYMTAB)
      continue;
    char const *shname = get_shstr(eh, sh->sh_name);
    if(!strcmp(shname, ".symtab"))
      return sh;
  }
  fprintf(stderr, "Object file's symbol table not found.\n");
  exit(1);
}

char const *get_shstr(Elf64_Ehdr *eh, Elf64_Word name)
{
  if(eh->e_shstrndx == SHN_UNDEF)
    return EMPTY_STR;
  Elf64_Shdr *shstrtab_sh = 0;
  char *shstrtab = 0;
  if(eh->e_shstrndx == SHN_XINDEX)
  {
    Elf64_Shdr *sh0 = (void *) eh + eh->e_shoff;
    if(sh0->sh_link == SHN_UNDEF)
      return EMPTY_STR;
    shstrtab_sh = (void *) eh + eh->e_shoff + sh0->sh_link * eh->e_shentsize;
  }
  else
  {
    shstrtab_sh = (void *) eh + eh->e_shoff + eh->e_shstrndx * eh->e_shentsize;
  }

  shstrtab = (void *) eh + shstrtab_sh->sh_offset;
  return shstrtab + name;
}
void *get_shcontent(Elf64_Ehdr *eh, Elf64_Shdr *sh)
{
  if(sh->sh_offset)
    return (void *) eh + sh->sh_offset;
  return 0;
}

int section_whitelisted(char const *name)
{
  uint64_t hash = elf_hash(name);
  for(size_t i = 0; i < WHITELISTED_SECTIONS_LEN; ++i)
  {
    if(whiteliested_sections_hash[i] == hash)
      if(!strcmp(name, whitelisted_sections[i]))
        return 1;
  }
  return 0;
}

int sh_isalloc(Elf64_Shdr *sh)
{
  return sh->sh_flags & SHF_ALLOC;
}

Elf64_Shdr *verify_shdr(Elf64_Ehdr *eh, Elf64_Word ndx)
{
  Elf64_Shdr *sh = getsh(eh, ndx);

  char const *shname = get_shstr(eh, sh->sh_name);

  if(!section_whitelisted(shname))
  {
    fprintf(
      stderr,
      "ELF file contains unknown section: '%s'\n",
      shname
    );
    exit(1);
  }

  return sh;
}

Elf64_Shdr *getsh(Elf64_Ehdr *eh, Elf64_Word ndx)
{
  if(ndx == SHN_UNDEF)
    return 0;
  return (void *) eh + eh->e_shoff + ndx * eh->e_shentsize;
}

Elf64_Sym *getsym(Elf64_Ehdr *eh, Elf64_Shdr *symsh, Elf64_Word ndx)
{
  return (void *) eh + symsh->sh_offset + ndx * symsh->sh_entsize;
}
Elf64_Sym *getsymn(Elf64_Ehdr *eh, char const *name)
{
  Elf64_Shdr *symtab = getsymtab(eh);
  size_t symcount = symtab->sh_size / symtab->sh_entsize;
  char const *strtab = getstrtab(eh);
  for(size_t i = 0; i < symcount; ++i)
  {
    Elf64_Sym *sym = getsym(eh, symtab, i);
    if(!sym)
      continue;
    char const *symname = strtab + sym->st_name;
    if(!strcmp(symname, "module_init"))
      return sym;
  }
  return 0;
}
Elf64_Rela *getrela(Elf64_Ehdr *eh, Elf64_Shdr *relash, Elf64_Word ndx)
{
  return (void *) eh + relash->sh_offset + ndx * relash->sh_entsize;
}
char const *getshname(Elf64_Ehdr *eh, Elf64_Word ndx)
{
  Elf64_Shdr *sh = getsh(eh, ndx);
  if(!sh)
    return 0;
  return get_shstr(eh, sh->sh_name);
}
