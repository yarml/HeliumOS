#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>
#include <elf.h>
#include <mem.h>

void kmod_load(void *kmodf)
{
  // Loading a kernel module is a multi step process consisting of:
  // - Loading into memory sections with flag SHF_ALLOC
  // - Applying relocations

   tpd("#### Loading Kernel module ####\n");

  size_t entrypoint = ((elf64_header *) kmodf)->entrypoint;

  printd("Entry point: %p\n", entrypoint);

  size_t phtl = elf_proght_len(kmodf);

  // Display all headers, for debugging
  for(size_t i = 0; i < phtl; ++i)
  {
    elf64_prog_header *ph = elf_prog_hdr(kmodf, i);
    printd("Program header #%lu\n");
    printd("\t\tType: %lu\n", ph->type);
    printd("\t\tVadr: %p\n", ph->vadr);
    printd("\t\tPadr: %p\n", ph->padr);
    printd("\t\tOffset: %p\n", ph->offset);
    printd("\t\tFile size: %lz\n", ph->file_size);
    printd("\t\tMem size: %lz\n", ph->mem_size);
    printd("\t\tFlags: %032b\n", ph->flags);
    printd("\t\tAlign: %lz\n", ph->align);
  }

  // Calculate total size to load
  size_t toload = 0;
  for(size_t i = 0; i < phtl; ++i)
  {
    elf64_prog_header *ph = elf_prog_hdr(kmodf, i);

    if(ph->type == PT_LOAD)
    {
      if(ph->align > MEM_PS || MEM_PS % ph->align)
        error_inv_state("Unsupported alignment for kernel module\n");
      printd("Adding %lz to load\n", ALIGN_UP(ph->mem_size, MEM_PS));
      toload += ALIGN_UP(ph->mem_size, MEM_PS);
    }
  }

  printd("In total %lz bytes need to be allocated in memory\n", toload);

  void *base = alloc_block(toload, MAPF_R | MAPF_W | MAPF_X);
  if(base == 0)
    error_out_of_memory("Could not allocate memory to load kernel module\n");
}

void kmod_uload()
{

}
