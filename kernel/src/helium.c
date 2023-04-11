#include <stdlib.h>
#include <string.h>
#include <utils.h>
#include <stdio.h>
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

  printf("File header:\n");
  printf("\tMachine: %02x\n", fptr->machine);
  printf("\tType: %02x\n", fptr->type);
  printf("\tEntrypoint: %p\n", fptr->entrypoint);

  printf("Program headers:\n");
  for(size_t i = 0; i < fptr->pht_len; ++i)
  {
    elf64_prog_header *ph = (void *) fptr + fptr->phoff + i * fptr->phent_size;
    printf("\tProgram header #%lu:\n", i);
    printf("\tType: %04x\n", ph->type);
    printf("\tMachine: %02x\n", fptr->machine);
  }

  printf("Section headers:\n");
  for(size_t i = 0; i < fptr->sht_len; ++i)
  {
    elf64_sect_header *sh = (void *) fptr + fptr->shoff + i * fptr->shent_size;
    printf("\tSection header #%lu\n", i);
    printf("\t\tName: '%s'\n", shstrtab + sh->name);
    printf("\t\tSize: %lu\n", sh->size);
    printf("\t\tOffset: %p\n", sh->offset);
    printf("\t\tType: %x\n", sh->type);
    printf("\t\tFlags: %lx\n", sh->flags);
    printf("\t\tLink: %u\n", sh->link);
    printf("\t\tInfo: %u\n", sh->info);

    if(sh->type == SHT_SYMTAB)
    {
      elf64_sect_header *tsh = (void *) fptr + fptr->shoff + sh->info * fptr->shent_size;
      printf("\t\tSymbols for '%s':\n", shstrtab + tsh->name);
      size_t sym_count = sh->size / sh->ent_size;
      for(size_t j = 0; j < sym_count; ++j)
      {
        elf64_sym *sym = (void *) fptr + sh->offset + j * sh->ent_size;
        elf64_sect_header *symsh = (void *) fptr + fptr->shoff + sym->shidx * fptr->shent_size;
        printf("\t\t\tSymbol #%lu:\n", j);
        printf("\t\t\t\tName: '%s'\n", strtab + sym->name);
        printf("\t\t\t\tValue: %p\n", sym->value);
        printf("\t\t\t\tSize: %lu\n", sym->size);
        printf("\t\t\t\tInfo: %02x\n", sym->info);
        printf("\t\t\t\tVisibility: %02x\n", sym->other);
        printf("\t\t\t\tSection: '%s'\n", shstrtab + symsh->name);

        if(!strcmp("module_init", strtab + sym->name))
        {
          void *vsec = alloc_block(symsh->size, MAPF_X);
          memcpy(vsec, (void *) fptr + symsh->offset, symsh->size);
          int (*mod_init)() = vsec + sym->value;
          int a = mod_init();
          printf("module_init()=%d\n", a);
          free_block(vsec, symsh->size);
        }
      }
    }
    else if(sh->type == SHT_RELA)
    {
      elf64_sect_header *tsh = (void *) fptr + fptr->shoff + sh->info * fptr->shent_size;
      printf("\t\tRelocation for: '%s'\n", shstrtab + tsh->name);
      size_t rel_count = sh->size / sh->ent_size;
      for(size_t j = 0; j < rel_count; ++j)
      {
        elf64_rela *rel = (void *) fptr + sh->offset + j * sh->ent_size;
        printf("\t\t\tRelocation #%lu:\n", j);
        printf("\t\t\t\tOffset: %p\n", rel->offset);
        printf("\t\t\t\tInfo: %016lx\n", rel->info);
        printf("\t\t\t\tAddend: %ld\n", rel->addend);
#define ELF64_R_SYM(info)             ((info)>>32)
#define ELF64_R_TYPE(info)            ((uint32_t)(info))
        printf("\t\t\t\tSymbol: %016lx\n", ELF64_R_SYM(rel->info));
        printf("\t\t\t\tType: %016lx\n", ELF64_R_TYPE(rel->info));
      }
    }
  }
  return 0;
}
