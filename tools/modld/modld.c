/*
  HeliumOS kernel module linker:

    Kernel modules for HeliumOS need special treatment when linked
  this tool is to be used to generate a kernel module from a relocatable file
    If the kernel module has multiple source files, they should be first
  combined into one relocatable file using `ld -r` then passed to this tools
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

  mod_ctx *mctx = mod_ctx_create();
  Elf64_Ehdr *eh = verify_ehdr(frel_data, frel_size);

  for(size_t i = 0; i < eh->e_shnum; ++i)
  {
    if(i == SHN_UNDEF)
      continue;
    Elf64_Shdr *sh = verify_shdr(eh, i);
    size_t shsize = sh->sh_size;

    char const *shname = get_shstr(eh, sh->sh_name);
    void const *shcontent = get_shcontent(eh, sh);

    if(!shsize)
      continue;
    if(sh_isalloc(sh)) // This should only be .text, .data, .rodata & .bss
    {
      switch(sh->sh_type)
      {
        case SHT_PROGBITS:
          mod_add_alloc_section(
            mctx, shname,
            shcontent, sh->sh_size,
            sh->sh_flags, sh->sh_addralign
          );
          break;
        case SHT_NOBITS:
          mod_add_alloc_nobits(
            mctx, shname,
            sh->sh_size, sh->sh_flags,
            sh->sh_addralign
          );
          break;
        default:
          fprintf(
            stderr,
            "Unsupported allocatable section type %08x for '%s'",
            sh->sh_type, shname
          );
          exit(1);
      }
    }
  }

  // Find entrypoint image offset
  size_t entrypoint_off = 0;
  Elf64_Shdr *symtab = getsymtab(eh);
  size_t symcount = symtab->sh_size / symtab->sh_entsize;
  char const *strtab = getstrtab(eh);
  Elf64_Sym *modinit_sym = 0;
  // First, find the symbol 'module_init'
  for(size_t i = 0; i < symcount; ++i)
  {
    Elf64_Sym *sym = getsym(eh, symtab, i);
    if(!sym)
      continue;
    char const *symname = strtab + sym->st_name;
    if(!strcmp(symname, "module_init"))
    {
      if(ELF64_ST_TYPE(sym->st_info) != STT_FUNC)
      {
        fprintf(stderr, "Symbol 'module_init' is not a function\n");
        exit(1);
      }
      modinit_sym = sym;
      break;
    }
  }
  if(!modinit_sym)
  {
    fprintf(stderr, "Could not find 'module_init' symbol\n");
    exit(1);
  }
  // Now, find the memory offset of the section module_init is in
  Elf64_Shdr *modinitsh = getsh(eh, modinit_sym->st_shndx);
  if(!modinitsh)
  {
    fprintf(stderr, "'module_init' is not associated with a section\n");
    exit(1);
  }
  char const *modinit_shname = get_shstr(eh, modinitsh->sh_name);
  size_t modinit_off = mod_section_moff(mctx, modinit_shname);
  if(modinit_off == SIZE_MAX)
  {
    fprintf(stderr, "'module_init' is defined in an unloadable section\n");
    exit(1);
  }
  entrypoint_off = modinit_off + modinit_sym->st_value;

  mod_genfile(mctx, entrypoint_off, fmod);

  free(frel_data);
  fclose(fmod);
  mod_ctx_destroy(mctx);
}
