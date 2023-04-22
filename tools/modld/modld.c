/*
  HeliumOS kernel module linker:

    Kernel modules for HeliumOS need special treatment when linked
  this too is to be used to generate a kernel module from a relocatable file
    If the kernel module has multiple source files, they should be first
  combined into one relocatable file using `ld -r` then passed
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <elf.h>

#include "modld.h"

int main(int argc, char **argv)
{
  if(argc != 3)
  {
    fprintf(stderr, "Usage: %s <relocatable-file> <module-file>\n", argv[0]);
    exit(1);
  }

  FILE *frel = fopen_or_exit(argv[1], "rb");
  FILE *fmod = fopen_or_exit(argv[2], "wb");

  elf_parser_init();

  size_t frel_size;

  fseek(frel, 0, SEEK_END);
  frel_size = ftell(frel);
  fseek(frel, 0, SEEK_SET);

  void *frel_data = calloc_or_exit(1, frel_size);

  if(fread(frel_data, 1, frel_size, frel) < frel_size)
  {
    fprintf(stderr, "Could not read all data from relocatable file.\n");
    exit(1);
  }

  fclose(frel);

  Elf64_Ehdr *eh = verify_ehdr(frel_data, frel_size);

  char const *strtab = getstrtab(eh);

  for(size_t i = 0; i < eh->e_shnum; ++i)
  {
    if(i == SHN_UNDEF)
      continue;
    Elf64_Shdr *sh = verify_shdr(eh, i);
    char const *shname = get_shstr(eh, sh->sh_name);

    if(!strcmp(shname, ".comment")) /* Discard .comment */
      continue;
    if(sh->sh_type == SHT_RELA)
    {
      Elf64_Shdr *targetsh = getsh(eh, sh->sh_info);
      Elf64_Shdr *symsh = getsh(eh, sh->sh_link);

      if(!targetsh)
      {
        fprintf(
          stderr,
          "Unspecified target for relocation section '%s'\n",
          shname
        );
        exit(1);
      }
      if(!symsh)
      {
        fprintf(
          stderr,
          "Unspecified symbol tavle for relocation section '%s'\n",
          shname
        );
      }

      char const *targetname = get_shstr(eh, targetsh->sh_name);
      char const *symshname = get_shstr(eh, symsh->sh_name);

      printf(
        "'%s': Relocations for section '%s': Symbols '%s'\n",
        shname, targetname, symshname
      );

      size_t rela_count = sh->sh_size / sh->sh_entsize;
      for(size_t r = 0; r < rela_count; ++r)
      {
        Elf64_Rela *rela = getrela(eh, sh, r);
        Elf64_Sym *sym = getsym(eh, symsh, ELF64_R_SYM(rela->r_info));
        Elf64_Shdr *symsh2 = getsh(eh, sym->st_shndx);
        printf("\tRelocation #%zu:\n", r);
        printf("\t\tOffset: %#016lx\n", rela->r_offset);
        printf("\t\tInfo: %#016lx\n", rela->r_info);
        printf("\t\t\tType:%#08lx\n", ELF64_R_TYPE(rela->r_info));
        printf("\t\t\tSymbol %lu:\n", ELF64_R_SYM(rela->r_info));
        printf("\t\t\t\tName: '%s'\n", strtab + sym->st_name);
        printf("\t\t\t\tValue: %#016lx\n", sym->st_value);
        printf("\t\t\t\tSize: %#016lx\n", sym->st_size);
        printf("\t\t\t\tVisibility: %#02x\n", sym->st_other);
        printf("\t\t\t\tInfo: %#016x\n", sym->st_info);
        printf("\t\t\t\t\tType: %#01x\n", ELF64_ST_TYPE(sym->st_info));
        printf("\t\t\t\t\tBind: %#016x\n", ELF64_ST_BIND(sym->st_info));
        if(symsh2)
          printf("\t\t\tSection: '%s'\n", get_shstr(eh, symsh2->sh_name));
        else
          printf("\t\t\tUndefined symbol\n");
      }
    }
  }

  free(frel_data);
  fclose(fmod);
}
