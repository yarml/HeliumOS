#ifndef HELIUM_INT_MEM_H
#define HELIUM_INT_MEM_H

#include <stddef.h>
#include <math.h>

#include <arch/mem.h>

extern void* i_pmm_header;
extern size_t i_mmap_usable_len;
extern mem_vpstruct_ptr* i_pml4;

#define BITMAP_SIZE(seg_size) (ALIGN_UP((seg_size) / MEM_PS, 64) / 8)

#define ADR_MASK(n)  (uintptr_t) ((uintptr_t) 0x1FF << (12 + 9 * (n)))
#define ADR_SHIFT(n) (size_t) (12 + 9 * ((n) + 1))

#define ENTRY_IDX(n, ptr) (size_t) (((uintptr_t) (ptr) & ADR_MASK(n)) >> ADR_SHIFT(n))

#define PML4_ORDER (3)
#define PDPT_ORDER (2)
#define PDT_ORDER  (1)
#define PT_ORDER   (0)

#define MAX_ORDER   (3)
#define ORDER_COUNT (MAX_ORDER + 1)

// TODO: I am unsure if this would work with page sizes different than 4K
// but hey, I only support x64 rn, so no worries
/* Trust me I did the math and it should work */
#define ORDER_STRUCT_SIZE(n) (size_t)(4096 * ((1 - powi(512, (n) + 1)) / ( 1 - 512)))


#endif