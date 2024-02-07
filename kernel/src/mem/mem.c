#include <boot_info.h>
#include <mem.h>
#include <proc.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <vcache.h>

#include <asm/ctlr.h>
#include <asm/gdt.h>
#include <asm/msr.h>
#include <asm/scas.h>

#include "internal_mem.h"

// Physical memory
void  *i_pmm_header      = 0;
void  *i_ppmm_header     = 0;
size_t i_mmap_usable_len = 0;

// Virtual memory
mem_pml4e *i_pmlmax  = 0;
mem_pml4e *i_ppmlmax = 0;

size_t i_order_ps[ORDER_COUNT] = {
    [0] = (size_t)MEM_PS,
    [1] = (size_t)512 * MEM_PS,
    [2] = (size_t)512 * 512 * MEM_PS,
    [3] = (size_t)512 * 512 * 512 * MEM_PS};

static gdt_entry basic_gdt[3];

static void basic_gdt_setup();

void mem_init() {
  prtrace_begin("mem_init", 0);
  /* Initializing memory is a long process, it should go this way
      - Setup our own gdt, do not rely on Bootboot's
      - Transform Bootboot's memory map into Helium's Memory map
      - Find a place in physical memory for the PMM bitmap
      - Initialize the VCache
      - Map the PMM bitmap into virtual space
      - Remove identitity mapping at [0;16G)
  */

  basic_gdt_setup();
  load_gdt(
      basic_gdt,
      sizeof(basic_gdt),
      MEM_KERNEL_DATA_DESC,
      MEM_KERNEL_CODE_DESC,
      0
  );

  size_t mmap_len = (bootboot.size - sizeof(BOOTBOOT)) / sizeof(MMapEnt) + 1;

  mem_pseg_header mmap_usable[mmap_len];

  size_t pmm_header_total_size = 0;

  MMapEnt *mmap = &(bootboot.mmap);

  /* Insert the regions backward, so that the PMM would prefer allocating
     pages with higher physical adresses and leave the lower addresses for
     hardware that requires it */
  for (size_t i = mmap_len; i != 0; --i) {
    printd(
        "mregion(type=%s, padr=%p, size= %lz)\n",
        mmap_type(MMapEnt_Type(mmap + i - 1)),
        (void *)ALIGN_UP(MMapEnt_Ptr(mmap + i - 1), MEM_PS),
        ALIGN_DN(MMapEnt_Size(mmap + i - 1), MEM_PS)
    );
    if (MMapEnt_IsFree(mmap + i - 1) && MMapEnt_Size(mmap + i - 1) >= MEM_PS) {
      mmap_usable[i_mmap_usable_len].magic = MEM_PSEG_MAGIC;
      mmap_usable[i_mmap_usable_len].padr =
          (void *)ALIGN_UP(MMapEnt_Ptr(mmap + i - 1), MEM_PS);
      mmap_usable[i_mmap_usable_len++].size =
          ALIGN_DN(MMapEnt_Size(mmap + i - 1), MEM_PS);
      pmm_header_total_size +=
          BITMAP_SIZE(MMapEnt_Size(mmap + i - 1)) + sizeof(mem_pseg_header);
    }
  }

  /* Search for a place for the PMM header */
  for (size_t i = 0; i < i_mmap_usable_len; ++i) {
    if (mmap_usable[i].size >= ALIGN_UP(pmm_header_total_size, MEM_PS)) {
      i_pmm_header = mmap_usable[i].padr;
      mmap_usable[i].padr += ALIGN_UP(pmm_header_total_size, MEM_PS);
      mmap_usable[i].size -= ALIGN_UP(pmm_header_total_size, MEM_PS);
      break;
    }
  }

  /* Initialize the PMM header */
  size_t pmm_header_off = 0;
  for (size_t i = 0; i < i_mmap_usable_len; ++i) {
    mem_pseg_header *h = i_pmm_header + pmm_header_off;
    *h                 = *(mmap_usable + i);
    memset((void *)h + sizeof(mem_pseg_header), 0, BITMAP_SIZE(h->size));
    /* Mark the leftover pages from the bitmap as used */
    uint64_t *last =
        (void *)h + sizeof(mem_pseg_header) + BITMAP_SIZE(h->size) - 8;
    *last = UINT64_MAX << (h->size / MEM_PS) % 64;
    pmm_header_off +=
        BITMAP_SIZE(mmap_usable[i].size) + sizeof(mem_pseg_header);

    printd(
        "header(header_adr=%016p,padr=%016p,size=%05lu)\n",
        h,
        h->padr,
        h->size / MEM_PS
    );
  }
  printd(
      "pmm header(adr=%016p,offset=%05lu, size=%05lu)\n",
      i_pmm_header,
      pmm_header_off,
      pmm_header_total_size
  );

  /* Initialize virtual memory manager */
  ctlr_cr3_npcid cr3 = as_rcr3();
  i_ppmlmax          = CTLR_CR3_NPCID_PML4_PADR(cr3);
  i_pmlmax           = i_ppmlmax;

  // Initialize VCache
  vcache_init();

  // Next we map the physical memory manager header into virtual space
  mem_vmap(PHEADER_VPTR, i_pmm_header, pmm_header_total_size, MAPF_R | MAPF_W);
  // Save the physical address in case it is needed
  i_pmm_header = PHEADER_VPTR;

  // Finally, we remove identity mapping setup by bootboot
  // TODO: we could reclaim the memory that the structures
  // used to identity map took.
  memset(i_pmlmax, 0, 256 * sizeof(mem_pml4e));
  as_rlcr3();

  prtrace_end("mem_init", 0, 0);
}

char const *mmap_type(uint8_t type) {
  switch (type) {
    default:
    case 0:
      return "MMAP_USED";
    case 1:
      return "MMAP_FREE";
    case 2:
      return "MMAP_ACPI";
    case 3:
      return "MMAP_MMIO";
  }
}

static void basic_gdt_setup() {
  // GDT 1 is kernel code segment
  basic_gdt[1].nsys    = 1;
  basic_gdt[1].exec    = 1;
  basic_gdt[1].dpl     = 0;
  basic_gdt[1].lmode   = 1;
  basic_gdt[1].present = 1;

  // GDT 2 is kernel data segment
  basic_gdt[2].nsys    = 1;
  basic_gdt[2].write   = 1;
  basic_gdt[2].dpl     = 0;
  basic_gdt[2].present = 1;
}

