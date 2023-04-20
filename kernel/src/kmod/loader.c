#include <string.h>
#include <stdio.h>
#include <utils.h>
#include <kmod.h>
#include <elf.h>
#include <mem.h>

kmod *kmod_load(void *kmodf)
{
  // Loading a kernel module is a multi step process consisting of:
  // - Loading into memory sections with flag SHF_ALLOC
  // - Applying relocations

  tpd("#### Loading Kernel module ####\n");

  elf64_header *eh = kmodf;

  size_t entrypoint = ((elf64_header *) kmodf)->entrypoint;

  printd("Entry point: %p\n", entrypoint);

  // Display all headers, for debugging
  for(size_t i = 0; i < eh->pht_len; ++i)
  {
    elf64_prog_header *ph = kmodf + eh->phoff + i * eh->phent_size;
    tpd("Program header #%lu\n", i);
    tpd("\tType: %s\n", elf_ptstr(ph->type));
    tpd("\tVadr: %p\n", ph->vadr);
    tpd("\tPadr: %p\n", ph->padr);
    tpd("\tOffset: %p\n", ph->offset);
    tpd("\tFile size: %lz\n", ph->file_size);
    tpd("\tMem size: %lz\n", ph->mem_size);
    tpd("\tFlags: %032b\n", ph->flags);
    tpd("\tAlign: %lz\n", ph->align);
  }

  uint64_t relaoff = 0;
  uint64_t relasz = 0;
  uint64_t relaent = 0;

  char *strtab = 0;

  uint64_t symtab_off = 0;
  uint64_t symtab_ent = 0;

  // Parse PT_DYNAMIC
  for(size_t i = 0; i < eh->pht_len; ++i)
  {
    elf64_prog_header *ph = kmodf + eh->phoff + i * eh->phent_size;

    if(ph->type == PT_DYNAMIC)
    {
      tpd("Dynamic segment #%lu\n", i);
      size_t dyn_count = ph->file_size / sizeof(elf64_dyn);
      elf64_dyn *dyns = kmodf + ph->offset;
      for(size_t j = 0; j < dyn_count; ++j)
      {
        tpd("\tDynamic entry #%lu\n", j);
        tpd("\t\tTag: %s\n", elf_dtstr(dyns[j].tag));
        tpd("\t\tVal/Ptr: %p\n", dyns[j].ptr);

        switch(dyns[j].tag)
        {
          case DT_RELA:
            relaoff = dyns[j].ptr - 0xffffffff80000000;
            break;
          case DT_RELASZ:
            relasz = dyns[j].val;
            break;
          case DT_RELAENT:
            relaent = dyns[j].val;
            break;
          case DT_SYMTAB:
            symtab_off = dyns[j].ptr - 0xffffffff80000000;
            break;
          case DT_SYMENT:
            symtab_ent = dyns[j].val;
            break;
          case DT_STRTAB:
            strtab = kmodf + dyns[j].ptr - 0xffffffff80000000;
            break;
        }
      }
    }
  }

  // if(symtab_ent)
  // {
  //   uint8_t *symtab = kmodf + symtab_off;
  //   size_t sym_count = symtab;
  // }

  // Calculate total size to load
  size_t toload = 0;
  void *base = 0;
  for(size_t i = 0; i < eh->pht_len; ++i)
  {
    elf64_prog_header *ph = kmodf + eh->phoff + i * eh->phent_size;

    if(ph->type == PT_LOAD)
    {
      if(ph->align > MEM_PS || MEM_PS % ph->align)
        error_inv_state("Unsupported alignment for kernel module\n");
      toload += ALIGN_UP(ph->mem_size, MEM_PS);
    }
  }
  base = mem_alloc_vblock(
    4 * toload,
    MAPF_R | MAPF_W | MAPF_X,
    KMOD_HEAP, KMOD_HEAP_SIZE
  );
  if(!base)
    error_out_of_memory("Could not allocate memory to load kernel module\n");

  tpd("Allocated %lz of memory for kernel module at %p\n", toload, base);

  for(size_t i = 0; i < eh->pht_len; ++i)
  {
    elf64_prog_header *ph = kmodf + eh->phoff + i * eh->phent_size;

    if(ph->type == PT_LOAD)
    {
      memcpy(base+(uintptr_t)ph->vadr - 0xffffffff80000000, kmodf+ph->offset, ph->file_size);
      memset(base+(uintptr_t)ph->vadr - 0xffffffff80000000 + ph->file_size, 0, ph->mem_size - ph->file_size);
    }
  }

  if(relaent)
  {
    size_t relacount = relasz / relaent;
    tpd("Applying relocations:\n");
    for(size_t i = 0; i < relacount; ++i)
    {
      elf64_rela *rela = kmodf + relaoff + i * relaent;
      elf64_sym *tsym = kmodf + symtab_off
          + ELF64_R_SYM(rela->info) * symtab_ent;

      tpd("Relocation #%lu\n", i);
      tpd("\t\tType: %s\n", elf_rtstr(ELF64_R_TYPE(rela->info)));
      tpd("\t\tOffset: %p\n", rela->offset);
      tpd("\t\tAddend: %lx\n", rela->addend);
      tpd("\t\tSymbol name: '%s'\n", strtab + tsym->name);

      switch(ELF64_R_TYPE(rela->info))
      {
        case R_AMD64_RELATIVE:
          uint64_t *target = base + rela->offset - 0xffffffff80000000;
          *target = (uintptr_t) base + rela->addend - 0xffffffff80000000;
          break;
        case R_AMD64_NONE:
          break;
        default:
          error_inv_state("Relocation type not supported.");
      }
    }
  }

  int (*mod_init)() = base + entrypoint - 0xffffffff80000000;

  tpd("Calling mod_init: %p.\n", mod_init);
  int ret = mod_init();

  tpd("Return value: %d\n", ret);

  return 0;
}

void kmod_uload(kmod *mod)
{
  
}
