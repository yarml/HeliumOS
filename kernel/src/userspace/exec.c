#include <initrd.h>
#include <mem.h>
#include <stdio.h>
#include <string.h>
#include <userspace.h>
#include <utils.h>

int exec() {
  // WIP, for now we just load a dummy file from initrd
  // Runs on a single core and waits until program exists, returning it's return
  // value
  initrd_file *dummy = initrd_search("/dummy");

  mem_vseg vseg = mem_alloc_vblock(
      dummy->size, MAPF_X, USERSPACE_EXEC_VPTR, USERSPACE_MAINEXEC_SIZE
  );
  if (vseg.error) {
    printd("Could not allocate space for executable\n");
    return -1;
  }
  memcpy(vseg.ptr, dummy->content, dummy->size);

  return ((int (*)())vseg.ptr)();
}