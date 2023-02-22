#include <boot_info.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>
#include <mem.h>

#include <asm/scas.h>
#include <asm/ctlr.h>
#include <asm/msr.h>

#include "internal_mem.h"
#include "vcache/vcache.h"

// Physical memory
void *i_pmm_header = 0;
size_t i_mmap_usable_len = 0;

// Virtual memory
mem_pml4e *i_pmlmax = 0;
mem_pml4e *i_ppmlmax = 0;

size_t i_order_ps[ORDER_COUNT];

void mem_init()
{
  printf("begin mem_init()\n");

  /* precalculate order page sizes */
  // TODO: figure a way to do it at compile time
  for(size_t i = 0; i < ORDER_COUNT; ++i)
  {
    i_order_ps[i] = ORDER_PS(i);
  }

  size_t mmap_len = (bootboot.size - sizeof(BOOTBOOT)) / sizeof(MMapEnt) + 1;

  mem_pseg_header mmap_usable[mmap_len];

  size_t pmm_header_total_size = 0;

  MMapEnt *mmap = &(bootboot.mmap);

  /* Insert the regions backward, so that the PMM would prefer allocating
     pages with higher physical adresses and leave the lower addresses for
     hardware that requires it */
  for(size_t i = mmap_len; i != 0; --i)
  {
    if(MMapEnt_IsFree(mmap + i - 1) && MMapEnt_Size(mmap + i - 1) >= MEM_PS)
    {
      mmap_usable[i_mmap_usable_len].padr =
        (void*) ALIGN_UP(MMapEnt_Ptr(mmap + i - 1), MEM_PS);
      mmap_usable[i_mmap_usable_len++].size =
        ALIGN_DN(MMapEnt_Size(mmap + i - 1), MEM_PS);
      pmm_header_total_size += BITMAP_SIZE(MMapEnt_Size(mmap + i - 1))
                            + sizeof(mem_pseg_header);
    }
  }

  /* Search for a place for the PMM header */
  for(size_t i = 0; i < i_mmap_usable_len; ++i)
  {
    if(mmap_usable[i].size >= ALIGN_UP(pmm_header_total_size, MEM_PS))
    {
      i_pmm_header = mmap_usable[i].padr;
      mmap_usable[i].padr += ALIGN_UP(pmm_header_total_size, MEM_PS);
      mmap_usable[i].size -= ALIGN_UP(pmm_header_total_size, MEM_PS);
      break;
    }
  }

  /* Initialize the PMM header */
  size_t pmm_header_off = 0;
  for(size_t i = 0; i < i_mmap_usable_len; ++i)
  {
    mem_pseg_header *h = i_pmm_header + pmm_header_off;
    *h = *(mmap_usable + i);
    memset((void*) h + sizeof(mem_pseg_header), 0, BITMAP_SIZE(h->size));
    /* Mark the leftover pages from the bitmap as used */
    uint64_t* last = (void*) h + sizeof(mem_pseg_header)
                               + BITMAP_SIZE(h->size) - 8;
    *last = UINT64_MAX << (h->size / MEM_PS) % 64;
    pmm_header_off += BITMAP_SIZE(mmap_usable[i].size)
                   + sizeof(mem_pseg_header);

    printf(
      "header(header_adr=%016p,padr=%016p,size=%05lu)\n",
      h, h->padr, h->size / MEM_PS
    );
  }
  printf("pmm header(adr=%016p,offset=%05lu)\n", i_pmm_header, pmm_header_off);

  /* Initialize virtual memory manager */
  ctlr_cr3_npcid cr3 = as_rcr3();
  i_ppmlmax = CTLR_CR3_NPCID_PML4_PADR(cr3);
  i_pmlmax = i_ppmlmax;

  // Initialize VCache
  vcache_init();

  printf("test mem_vmap()\n");

  mem_vmap(
    KVMSPACE + (uint64_t) 0x8000000000,
    (void *) 0x4000,
    0x1000,
    0
  );

  mem_vmap(
    KVMSPACE + (uint64_t) 0x8000000000 + 0x2000,
    (void *) 0x4000,
    0x1000,
    0
  );

  printf("end mem_init()\n");
}
