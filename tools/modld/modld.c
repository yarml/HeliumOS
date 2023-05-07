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

  // Verify all section headers are supported & add allocatable sections to
  // the module context
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
    if(!sh_isalloc(sh)) // This should only be .text, .data, .rodata & .bss
      continue;
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

  // Find entrypoint image offset
  size_t entrypoint_off = 0;
  Elf64_Sym *modinit_sym = getsymn(eh, "module_init");
  if(!modinit_sym)
  {
    fprintf(stderr, "Could not find symbol 'module_init'\n");
    exit(1);
  }
  // Now, find the memory offset of the section module_init is in
  char const *modinit_shname = getshname(eh, modinit_sym->st_shndx);
  if(!modinit_shname)
  {
    fprintf(stderr, "Could not find the section of 'module_init'\n");
    exit(1);
  }
  if(modinit_sym->st_shndx >= SHN_LORESERVE)
  {
    fprintf(stderr, "Unsupported symbol type of 'module_init'\n");
    exit(1);
  }
  entrypoint_off = mod_symoff(mctx, modinit_shname, modinit_sym->st_value);
  if(entrypoint_off == SIZE_MAX)
  {
    fprintf(stderr, "'module_init' is defined in an unloadable section\n");
    exit(1);
  }

  // Loop through all relocations and apply the ones that we have enough
  // information for
  for(size_t i = 0; i < eh->e_shnum; ++i)
  {
    if(i == SHN_UNDEF)
      continue;
    Elf64_Shdr *sh = getsh(eh, i);
    size_t shsize = sh->sh_size;

    if(!shsize)
      continue;
    if(sh->sh_type == SHT_REL)
    {
      fprintf(stderr, "Unsupported relocation type REL\n");
      exit(1);
    }
    if(sh->sh_type != SHT_RELA)
      continue;
    char const *targetshname = getshname(eh, sh->sh_info);
    Elf64_Shdr *rela_symtab = getsh(eh, sh->sh_link);
    char const *strtab = getstrtab(eh);
    size_t relacount = shsize / sh->sh_entsize;
    for(size_t j = 0; j < relacount; ++j)
    {
      Elf64_Rela *rela = getrela(eh, sh, j);
      Elf64_Sym *target_sym =
        getsym(eh, rela_symtab, ELF64_R_SYM(rela->r_info));
      if(target_sym->st_shndx == SHN_UNDEF)
      {
        switch(ELF64_R_TYPE(rela->r_info))
        {
          case R_X86_64_PLT32:
          {
            mod_refjte(
              mctx,
              strtab + target_sym->st_name,
              targetshname,
              rela->r_offset
            );
          }
            break;
          case R_X86_64_REX_GOTPCRELX:
          {
            mod_refkgote(
              mctx,
              targetshname,
              rela->r_offset,
              strtab + target_sym->st_name
            );
          }
            break;
          default:
            fprintf(
              stderr,
              "Unsupported undefined symbol relocation %lx\n",
              ELF64_R_TYPE(rela->r_info)
            );
            exit(1);
        }
        continue; // Not enough information to apply this relocation
      }
      size_t symval;
      if(target_sym->st_shndx < SHN_LORESERVE)
      {
        char const *symshname = getshname(eh, target_sym->st_shndx);
        symval = mod_symoff(mctx, symshname, target_sym->st_value);
        if(symval == SIZE_MAX)
        {
          fprintf(
            stderr,
            "Could not determine value of symbol %s\n",
            strtab + target_sym->st_name
          );
          exit(1);
        }
      }
      else if(target_sym->st_shndx == SHN_ABS)
        symval = target_sym->st_value;
      else
      {
        fprintf(
          stderr,
          "Unsupported section index %x for symbol '%s'\n",
          target_sym->st_shndx, strtab + target_sym->st_name
        );
        exit(1);
      }

      uint8_t patch[8];
      int patch_size = 0;

      switch(ELF64_R_TYPE(rela->r_info))
      {
        case R_X86_64_PC32:
        {
          uint32_t val = symval + rela->r_addend - rela->r_offset;
          patch_size = 4;
          memcpy(patch, &val, 4);
        }
          break;
        case R_X86_64_PLT32: // Kernel module calling its own function
        {
          uint32_t off = symval - rela->r_offset - rela->r_addend;
          patch_size = 4;
          memcpy(patch, &off, 4);
        }
          break;
        case R_X86_64_REX_GOTPCRELX: // Kernel module accessing its own data
        {
          // We cannot do the patch now
          patch_size = 0;
          // We will let mod_builder do it when generating the output file
          mod_refgote(
            mctx,
            targetshname,
            rela->r_offset,
            symval
          );
        }
          break;
        default:
          fprintf(
            stderr,
            "Unsupported relocation type %lx\n",
            ELF64_R_TYPE(rela->r_info)
          );
          exit(1);
      }

      void *shcontent = mod_section_content(mctx, targetshname);
      if(!shcontent)
      {
        fprintf(
          stderr,
          "Relocation to unallocated section '%s'\n",
          targetshname
        );
        exit(1);
      }

      memcpy(shcontent + rela->r_offset, patch, patch_size);
    }
  }

  mod_genfile(mctx, entrypoint_off, fmod);

  free(frel_data);
  fclose(fmod);
  mod_ctx_destroy(mctx);
}
