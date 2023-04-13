/*
  This tool generates a linker script that defines symbols from
  an ELF file.

  This is used in HeliumOS to link kernel modules with the ability
  to access core kernel symbols.
*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <elf.h>

int main(int argc, char **argv)
{
  if(argc != 4)
  {
    fprintf(stderr, "Usage: %s <ELF-file> <ld-output-file> <footer>\n", argv[0]);
    return 1;
  }

  FILE *elf = fopen(argv[1], "r");
  if(!elf)
  {
    fprintf(stderr, "Could not open ELF file '%s'.\n", argv[1]);
    return 1;
  }

  FILE *ld = fopen(argv[2], "w");
  if(!ld)
  {
    fprintf(stderr, "Could not open linker script file '%s'.\n", argv[2]);
    return 2;
  }
  FILE *footer = fopen(argv[3], "r");
  if(!footer)
  {
    fprintf(stderr, "Could not open linker script footer '%s'.\n", argv[3]);
    return 3;
  }

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

  fprintf(ld, "/* Auto generated symbol definitions by '%s' */\n\n", argv[0]);

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

      // If the name is empty skip this symbol.
      if(!strlen(name))
        continue;

      // If the symbol is not global, it should not
      // be outputed to the linker script.
      if(ELF64_ST_BIND(sym->st_info) != STB_GLOBAL)
        continue;

      fprintf(ld, "%s = %#0lx;\n", name, val);
    }
  }

  fprintf(ld, "\n/* Footer from '%s'*/\n", argv[3]);

  // Now we append the module linker script footer
  char buf[32];
  while(!feof(footer))
  {
    size_t read = fread(buf, 1, sizeof(buf), footer);
    fwrite(buf, 1, read, ld);
  }
}