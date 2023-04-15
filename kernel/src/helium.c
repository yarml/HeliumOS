#include <stdlib.h>
#include <string.h>
#include <utils.h>
#include <stdio.h>
#include <kmod.h>
#include <elf.h>
#include <mem.h>
#include <fs.h>

#include <fs/tar.h>

int kmain()
{
  // loading modules, and starting the init process should be done here

  fsnode *fksym = fs_search("initrd://modules/test.mod");
  // size_t fsize = fs_tellsize(fksym);
  elf64_header *fptr = tarfs_direct_access(fksym);
  fs_close(fksym);

  elf64_sect_header *str_sh =
      (void *) fptr + fptr->shoff + fptr->shstridx * fptr->shent_size;

  char *shstrtab = (void *)fptr + str_sh->offset;
  char *strtab = 0;

  // Search string table
  for(size_t i = 0; i < fptr->sht_len; ++i)
  {
    elf64_sect_header *sh = (void *) fptr + fptr->shoff + i * fptr->shent_size;
    if(!strcmp(shstrtab + sh->name, ".strtab"))
    {
      strtab = (void *) fptr + sh->offset;
      break;
    }
  }

  printd("File header:\n");
  printd("\tMachine: %02x\n", fptr->machine);
  printd("\tType: %02x\n", fptr->type);
  printd("\tEntrypoint: %p\n", fptr->entrypoint);

  printd("Program headers:\n");
  for(size_t i = 0; i < fptr->pht_len; ++i)
  {
    elf64_prog_header *ph = (void *) fptr + fptr->phoff + i * fptr->phent_size;
    printd("\tProgram header #%lu:\n", i);
    printd("\t\tType: %04x\n", ph->type);
    printd("\t\tVadr: %p\n", ph->vadr);
    printd("\t\tPadr: %p\n", ph->padr);
    printd("\t\tOffset: %p\n", ph->offset);
    printd("\t\tFile size: %lz\n", ph->file_size);
    printd("\t\tMem size: %lz\n", ph->mem_size);
    printd("\t\tAlign: %lz\n", ph->align);
  }

  printd("Section headers:\n");
  for(size_t i = 0; i < fptr->sht_len; ++i)
  {
    elf64_sect_header *sh = (void *) fptr + fptr->shoff + i * fptr->shent_size;
    printd("\tSection header #%lu\n", i);
    printd("\t\tName: '%s'\n", shstrtab + sh->name);
    printd("\t\tSize: %lz\n", sh->size);
    printd("\t\tOffset: %p\n", sh->offset);
    printd("\t\tType: %x\n", sh->type);
    printd("\t\tFlags: %lx\n", sh->flags);
    printd("\t\tLink: %u\n", sh->link);
    printd("\t\tInfo: %u\n", sh->info);

    if(sh->type == SHT_SYMTAB || sh->type == SHT_DYNSYM)
    {
      elf64_sect_header *tsh = (void *) fptr + fptr->shoff + sh->info * fptr->shent_size;
      printd("\t\tSymbols for '%s':\n", shstrtab + tsh->name);
      size_t sym_count = sh->size / sh->ent_size;
      for(size_t j = 0; j < sym_count; ++j)
      {
        elf64_sym *sym = (void *) fptr + sh->offset + j * sh->ent_size;
        printd("\t\t\tSymbol #%lu:\n", j);
        printd("\t\t\t\tName: '%s'\n", strtab + sym->name);
        printd("\t\t\t\tValue: %p\n", sym->value);
        printd("\t\t\t\tSize: %lu\n", sym->size);
        printd("\t\t\t\tInfo: %02x\n", sym->info);
        printd("\t\t\t\tVisibility: %02x\n", sym->other);
      }
    }
  }

  kmod_load(fptr);

  return 0;
}
