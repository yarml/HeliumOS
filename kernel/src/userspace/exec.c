#include <elf.h>
#include <initrd.h>
#include <mem.h>
#include <stdio.h>
#include <string.h>
#include <userspace.h>
#include <utils.h>

#include <asm/userspace.h>

static bool verify_elf(elf64_header *header) {
  return true;  // We trust them
}

void exec() {
  // WIP, for now we just load a dummy file from initrd
  // Runs on a single core and waits until program exists
  initrd_file  *dummy     = initrd_search("/bin/init");
  elf64_header *exec_file = dummy->content;

  if (!verify_elf(exec_file)) {
    printd("Invalid ELF File\n");
    return;
  }

  // Make the ELF image
  elf64_prog_header *ph = (void *)exec_file + exec_file->phoff;
  for (size_t i = 0; i < exec_file->pht_len; ++i) {
    if (ph->type != PT_LOAD) {
      printd("Unsupported program header directive\n");
      // FIXME: Dellaocate previously allocated blocks
      return;
    }
    if (ph->align > 0x1000) {
      printd("Unsupported alignment higher than 1 page\n");
      return;
    }

    int flags = MAPF_U;
    if (ph->flags & PF_W) {
      flags |= MAPF_W;
    }
    if (ph->flags & PF_R) {
      flags |= MAPF_R;
    }
    if (ph->flags & PF_X) {
      flags |= MAPF_X;
    }

    void  *vadr      = (void *)ph->vadr;
    void  *vadr_base = (void *)ALIGN_DN(ph->vadr, 0x1000);
    size_t real_size =
        ALIGN_UP(ph->mem_size + (uintptr_t)vadr - (uintptr_t)vadr_base, 0x1000);
    // size_t effective_size = ph->mem_size;

    mem_alloc_into(vadr_base, real_size, flags);
    memset(vadr_base, 0, real_size);
    memcpy(vadr, (void *)exec_file + ph->offset, ph->file_size);

    ph = (void *)ph + exec_file->phent_size;
  }

  // Allocate stack
  mem_alloc_into(USPACE_STACK_TOP, USPACE_STACK_SIZE, MAPF_W | MAPF_R | MAPF_U);
  as_call_userspace((void *)exec_file->entrypoint, USPACE_STACK_BASE, 0x200);
}