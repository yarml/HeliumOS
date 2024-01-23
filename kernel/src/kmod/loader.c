#include <elf.h>
#include <errno.h>
#include <fs.h>
#include <hashtable.h>
#include <kmod.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>

#include "internal_kmod.h"

kmod *kmod_loadp(char const *path) {
  fsnode *f = fs_search(path);
  if (!f) {
    return 0;
  }
  kmod *mod = kmod_loadf(f);
  fs_close(f);
  return mod;
}

kmod *kmod_loadf(fsnode *f) {
  size_t fsize = fs_tellsize(f);
  if (!fsize) {
    return 0;
  }
  char  *buf  = malloc(fsize);
  size_t read = 0;
  while (read < fsize) {
    errno     = 0;
    size_t cr = fs_read(f, read, buf + read, fsize - read);
    if (!cr && errno) {
      return 0;
    }
    read += cr;
  }
  kmod *mod = kmod_loadb(buf, f->name);
  free(buf);
  return mod;
}

kmod *kmod_loadb(void *kmodf, char name[KMOD_NAMELEN]) {
  elf64_header              *eh  = kmodf;
  elf64_prog_header         *ph  = kmodf + eh->phoff;
  elf64_kmod_loader_command *cmd = kmodf + ph->offset;
  size_t cmd_count = ph->file_size / sizeof(elf64_kmod_loader_command);

  mem_vseg kmod_vseg = mem_alloc_vblock(
      ph->mem_size,
      MAPF_W | MAPF_R | MAPF_X,  // TODO: Give 0 permissions by default
      KMOD_HEAP,
      KMOD_HEAP_SIZE
  );

  if (kmod_vseg.error) {
    return 0;
  }

  kmod *mod = calloc(1, sizeof(kmod));

  strcpy(mod->name, name);
  mod->vseg = kmod_vseg;

  void       *base   = kmod_vseg.ptr;
  char const *symtab = 0;

  for (size_t i = 0; i < cmd_count; ++i) {
    switch (cmd[i].command) {
      case CM_UNDEF:
        break;
      case CM_MAP:
        memcpy(
            base + cmd[i].mem.moff, kmodf + cmd[i].mem.foff, cmd[i].mem.size
        );
        break;
      case CM_ZMEM:
        memset(base + cmd[i].mem.moff, 0, cmd[i].mem.size);
        break;
      case CM_LDSYM:
        symtab = kmodf + cmd[i].mem.foff;
        break;
      case CM_JTE:
        if (!symtab) {
          error_inv_state("Module contains JTE instruction before LDSYM");
        }
        if (!i_ksym_table) {
          error_inv_state(
              "Module uses kernel symbols but "
              "kernel symbols are not loaded"
          );
        }
        {
          char const *symname    = symtab + cmd[i].patch.val;
          void       *patch_vadr = base + cmd[i].patch.patchoff;
          uint64_t   *symvalp    = hash_table_search(i_ksym_table, symname);
          if (!symvalp) {
            error_inv_state("Module uses unknown symbol");
          }
          uint64_t symval = *symvalp;
          uint32_t patch = symval - (uintptr_t)base - cmd[i].patch.patchoff - 4;
          memcpy(patch_vadr, &patch, sizeof(patch));
        }
        break;
      case CM_ADDBASE: {
        void    *patch_vadr = base + cmd[i].patch.patchoff;
        uint64_t patch      = (uintptr_t)base + cmd[i].patch.val;
        memcpy(patch_vadr, &patch, sizeof(patch));
      } break;
      case CM_KSYM:
        if (!symtab) {
          error_inv_state("Module contains JTE instruction before LDSYM");
        }
        if (!i_ksym_table) {
          error_inv_state(
              "Module uses kernel symbols but "
              "kernel symbols are not loaded"
          );
        }
        {
          char const *symname    = symtab + cmd[i].patch.val;
          void       *patch_vadr = base + cmd[i].patch.patchoff;
          uint64_t   *symvalp    = hash_table_search(i_ksym_table, symname);
          if (!symvalp) {
            error_inv_state("Module uses unknown symbol");
          }
          uint64_t patch = *symvalp;
          memcpy(patch_vadr, &patch, sizeof(patch));
        }
        break;
    }
  }
  int (*mod_init)() = base + eh->entrypoint;
  mod_init();
  return 0;
}

void kmod_uload(kmod *mod) {
}
