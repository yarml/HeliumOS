#include <boot_info.h>
#include <fs.h>
#include <initrd.h>
#include <mem.h>

#include <fs/tar.h>

#include "internal_fs.h"

void fs_init() {
  i_fs_head = 0;

  mem_vmap(
      INITRD_VPTR, (void *)bootboot.initrd_ptr, bootboot.initrd_size, MAPF_R
  );
  tar_mkimfs("initrd", INITRD_VPTR, bootboot.initrd_size);
}