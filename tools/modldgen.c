/*
  This tool generates a linker script that defines symbols from
  an ELF file.

  This is used in HeliumOS to link kernel modules with the ability
  to access core kernel symbols.
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <elf.h>

FILE *open_safe(char *path, char *mode)
{
  FILE *f = fopen(path, mode);
  if(!f)
  {
    fprintf(stderr, "Could not open file '%s'.\n", path);
    exit(1);
  }
}

void combine_name(char *out, char *p1, char *p2)
{
  out[0] = 0;
  strcat(out, p1);
  strcat(out, p2);
}

int main(int argc, char **argv)
{
  if(argc != 3 || !strlen(argv[1]) || !strlen(argv[2]))
  {
    fprintf(stderr, "Usage: %s <sym-file> <ld-output-file-suffix>\n", argv[0]);
    return 1;
  }

  FILE *elf = open_safe(argv[1], "r");

  size_t ldpref_len = strlen(argv[2]);

  char rodatald_path[ldpref_len+8];
  char textld_path[ldpref_len+6];
  char datald_path[ldpref_len+6];
  char absld_path[ldpref_len+5];
  char bssld_path[ldpref_len+5];
  char comld_path[ldpref_len+5];
  char unkld_path[ldpref_len+5];

  combine_name(rodatald_path, argv[2], ".rodata");
  combine_name(textld_path, argv[2], ".text");
  combine_name(datald_path, argv[2], ".data");
  combine_name(absld_path, argv[2], ".abs");
  combine_name(bssld_path, argv[2], ".bss");
  combine_name(comld_path, argv[2], ".com");
  combine_name(unkld_path, argv[2], ".unk");

  FILE *rodatald = open_safe(rodatald_path, "w");
  FILE *textld = open_safe(textld_path, "w");
  FILE *datald = open_safe(datald_path, "w");
  FILE *absld = open_safe(absld_path, "w");
  FILE *bssld = open_safe(bssld_path, "w");
  FILE *comld = open_safe(comld_path, "w");
  FILE *unkld = open_safe(unkld_path, "w");

  // Read symbol file header
  Elf64_Ehdr eh;
  fread(&eh, sizeof(eh), 1, elf);

  size_t sht_size = eh.e_shnum * eh.e_shentsize;
  fseek(elf, eh.e_shoff, SEEK_SET);

  uint8_t sht[sht_size];
  fread(sht, eh.e_shentsize, eh.e_shnum, elf);

  Elf64_Shdr *shstrtab_sh = (void *) sht + eh.e_shstrndx * eh.e_shentsize;

  // Load section header names from ELF file.
  uint8_t shstrtab[shstrtab_sh->sh_size];
  fseek(elf, shstrtab_sh->sh_offset, SEEK_SET);
  fread(shstrtab, 1, sizeof(shstrtab), elf);

  // Find string table
  Elf64_Shdr *strtab_sh = 0;
  for(size_t i = 0; i < eh.e_shnum; ++i)
  {
    Elf64_Shdr *sh = (void *) sht + i * eh.e_shentsize;

    if(sh->sh_type != SHT_STRTAB)
      continue;
    if(strcmp(".strtab", shstrtab + sh->sh_name))
      continue;

    // We found the string table.
    strtab_sh = sh;
    break;
  }

  if(!strtab_sh)
  {
    fprintf(
      stderr,
      "Could not find string table for Elf file '%s'.\n", argv[1]
    );
    return 1;
  }

  // Load string table from ELF file.
  uint8_t strtab[strtab_sh->sh_size];
  fseek(elf, strtab_sh->sh_offset, SEEK_SET);
  fread(strtab, 1, sizeof(strtab), elf);

  // Now find symbol table
  for(size_t i = 0; i < eh.e_shnum; ++i)
  {
    Elf64_Shdr *sh = (void *) sht + i * eh.e_shentsize;

    // Skip this section entry if it is not a symbol table
    if(sh->sh_type != SHT_SYMTAB)
      continue;

    size_t sym_count = sh->sh_size / sh->sh_entsize;

    // Load the symbol table from the ELF file
    uint8_t symtab[sh->sh_size];
    fseek(elf, sh->sh_offset, SEEK_SET);
    fread(symtab, sh->sh_entsize, sym_count, elf);

    // Loop for each symbol and output a line in the linker
    // script for that symbol.
    for(size_t j = 1; j < sym_count; ++j)
    {
      Elf64_Sym *sym = (void *) symtab + j * sh->sh_entsize;

      char *name = strtab + sym->st_name;
      uint64_t val = sym->st_value;

      char *sn;

      if(sym->st_shndx < SHN_LORESERVE)
      {
        Elf64_Shdr *ssh = (void *) sht + sym->st_shndx * eh.e_shentsize;
        sn = shstrtab + ssh->sh_name;
      }
      else
      {
        switch(sym->st_shndx)
        {
          case SHN_ABS:
            sn = "abs";
            break;
          case SHN_COMMON:
            sn = "common";
            break;
          default:
            sn = "unk";
        }
      }


      // If the name is empty skip this symbol.
      if(!strlen(name))
        continue;

      // If the symbol is not global, it should not
      // be outputed to the linker script.
      if(ELF64_ST_BIND(sym->st_info) != STB_GLOBAL)
        continue;

      FILE *of;

      if(!strcmp(sn, ".rodata"))
        of = rodatald;
      else if(!strcmp(sn, ".text"))
        of = textld;
      else if(!strcmp(sn, ".data"))
        of = datald;
      else if(!strcmp(sn, ".bss"))
        of = bssld;
      else if(!strcmp(sn, "abs"))
        of = absld;
      else if(!strcmp(sn, "common"))
        of = comld;
      else
        of = unkld;

      fprintf(of, "PROVIDE(%s = %#0lx);\n", name, val);
    }
  }
}