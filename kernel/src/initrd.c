#include <boot_info.h>
#include <initrd.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tar.h>
#include <utils.h>

#include <dts/hashtable.h>

static dts_hashtable *initrd_nodes;

void initrd_init() {
  mem_vmap(
      INITRD_VPTR, (void *)bootboot.initrd_ptr, bootboot.initrd_size, MAPF_R
  );

  initrd_nodes = dts_hashtable_create_strkey(0);

  tar_header *current_header = INITRD_VPTR;
  // FIXME: Assumes that directories always come before files
  while (1) {
    if ((uintptr_t)current_header + sizeof(tar_header) >
            (uintptr_t)INITRD_VPTR + bootboot.initrd_size ||
        memcmp(current_header->ustar, "ustar  ", 8)) {
      break;
    }

    tar_header *ch       = current_header;
    size_t      filesize = stou(ch->size, 0, 8);
    current_header =
        (void *)current_header + sizeof(tar_header) + ALIGN_UP(filesize, 512);

    if (ch->type != TAR_ENTRY_TYPE_FILE && ch->type != TAR_ENTRY_TYPE_DIR) {
      continue;
    }

    size_t name_len = strlen(ch->name);
    if (name_len > 100) {
      name_len = 100;
    }
    size_t namepref_len = strlen(ch->name_pref);
    if (namepref_len > 155) {
      namepref_len = 155;
    }

    size_t filename_cap = name_len + namepref_len + 2;
    char  *filename     = calloc(1, filename_cap);
    snprintf(filename, filename_cap, "/%s%s", ch->name_pref, ch->name);

    initrd_file *file = calloc(1, sizeof(initrd_file));
    file->path        = filename;
    file->size        = filesize;
    file->type        = ch->type;

    if (file->type == TAR_ENTRY_TYPE_FILE) {
      file->content = ch + 1;
    }
    printd("INITRD: '%s'\n", file->path);
    dts_hashtable_insert(initrd_nodes, file->path, file);
  }
}
initrd_file *initrd_search(char const *path) {
  return dts_hashtable_search(initrd_nodes, path, 0);
}
