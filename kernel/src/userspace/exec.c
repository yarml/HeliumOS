#include <initrd.h>
#include <mem.h>
#include <stdio.h>
#include <string.h>
#include <userspace.h>
#include <utils.h>

#include <asm/userspace.h>

int exec() {
  // WIP, for now we just load a dummy file from initrd
  // Runs on a single core and waits until program exists, returning it's return
  // value
  initrd_file *dummy = initrd_search("/dummy");

  mem_vseg code_vseg = mem_alloc_vblock(
      dummy->size, MAPF_X | MAPF_U, USERSPACE_EXEC_VPTR, USERSPACE_MAINEXEC_SIZE
  );
  if (code_vseg.error) {
    printd("Could not allocate space for executable\n");
    return -1;
  }
  memcpy(code_vseg.ptr, dummy->content, dummy->size);

  mem_vseg stack_vseg = mem_alloc_vblock(
      USPACE_STACK_SIZE,
      MAPF_W | MAPF_R | MAPF_U,
      USPACE_STACK_TOP,
      USPACE_STACK_SIZE
  );
  if (stack_vseg.error) {
    printd("Could not allocate stack for executable\n");
    return -1;
  }
  memset(stack_vseg.ptr, 0, USPACE_STACK_SIZE);

  void *stack = stack_vseg.ptr + USPACE_STACK_SIZE;

  printd("User code segment starts at: %p\n", code_vseg.ptr);
  printd("User stack segment starts at: %p\n", stack_vseg.ptr);

  as_call_userspace(code_vseg.ptr, stack, 0);
}