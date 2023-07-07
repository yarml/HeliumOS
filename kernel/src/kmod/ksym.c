// Load kernel symbols from file pointed to in path

#include <elf.h>
#include <errno.h>
#include <fs.h>
#include <hashtable.h>
#include <kmod.h>
#include <stdio.h>
#include <string.h>

hash_table *i_ksym_table = 0;

int         ksym_loadp(char const *path) {
  fsnode *f = fs_search(path);
  if (!f) return 1;
  int status = ksym_loadf(f);
  fs_close(f);
  return status;
}

int ksym_loadf(fsnode *f) {
  size_t fsize = fs_tellsize(f);
  if (!fsize) return 1;
  char   buf[fsize];
  size_t read = 0;
  while (read < fsize) {
    errno     = 0;
    size_t cr = fs_read(f, read, buf + read, fsize - read);
    if (!cr && errno) return 1;
    read += cr;
  }
  return ksym_loadb(buf);
}

int ksym_loadb(void *ksymf) {
  elf64_header      *eh = ksymf;

  elf64_sect_header *shstrtab_sh =
      ksymf + eh->shoff + eh->shstridx * eh->shent_size;
  char const        *shstrtab  = ksymf + shstrtab_sh->offset;
  char const        *strtab    = 0;
  elf64_sect_header *symtab_sh = 0;

  for (size_t i = 0; i < eh->sht_len; ++i) {
    elf64_sect_header *sh     = ksymf + eh->shoff + i * eh->shent_size;
    char const        *shname = shstrtab + sh->name;
    if (!strcmp(shname, ".strtab")) strtab = ksymf + sh->offset;
    if (!strcmp(shname, ".symtab")) symtab_sh = sh;
    if (strtab && symtab_sh) break;
  }

  if (!strtab || !symtab_sh) {
    printd("Could not find .strtab or .symtab in kernel symbol file\n");
    return 1;
  }

  hash_table *new_ksym_table = hash_table_create(sizeof(uint64_t));
  if (!new_ksym_table) return 1;

  size_t sym_count = symtab_sh->size / symtab_sh->ent_size;
  for (size_t i = 0; i < sym_count; ++i) {
    elf64_sym *sym = ksymf + symtab_sh->offset + i * symtab_sh->ent_size;
    if (!strlen(strtab + sym->name)) continue;
    uint64_t *val = hash_table_addkey(new_ksym_table, strtab + sym->name);
    if (!val) {
      hash_table_destroy(new_ksym_table);
      return 1;
    }
    *val = sym->value;
  }

  if (i_ksym_table) hash_table_destroy(i_ksym_table);
  i_ksym_table = new_ksym_table;

  return 0;
}
