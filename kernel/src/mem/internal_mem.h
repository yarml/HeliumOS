#ifndef HELIUM_INT_MEM_H
#define HELIUM_INT_MEM_H

#include <math.h>
#include <mem.h>
#include <stddef.h>

#include <arch/mem.h>

// TODO: find optimal value
// Threshold, if a virtual mapping needs to modify more than this much pages
// then reloading cr3 is prefered over invlpg (unless the paging is global)
#define RLCR3_THRESHOLD (32)

#define BITMAP_SIZE(seg_size) (ALIGN_UP((seg_size) / MEM_PS, 64) / 8)

#define ADR_MASK(n) (uintptr_t)((uintptr_t)0x1FF << (12 + 9 * (n)))
#define ADR_SHIFT(n) (size_t)(12 + 9 * (n))

// Gives the index of the entry in the parent structure of order n
// that leads to pointing out to ptr
#define ENTRY_IDX(n, ptr) \
  (size_t)(((uintptr_t)(ptr)&ADR_MASK(n)) >> ADR_SHIFT(n))

char const *mmap_type(uint8_t type);

// TODO: when porting to other architectures, this should be considered
// I'll probably never port, struggling enough with x86-64 lol
#define ORDER_COUNT (4)
#define MAX_ORDER (ORDER_COUNT - 1)

#define ORDER_PS(n) (i_order_ps[n])

extern void *i_pmm_header;
extern size_t i_mmap_usable_len;

// The physical address of the VMM structure of order MAX_ORDER(PML4)
// As set by bootboot
// It is also an array of 512 entries, obviously
extern mem_pml4e *i_ppmlmax;
// The virtual address of the VMM structure of order MAX_ORDER(PML4)
// As set by vcache
// It is also an array of 512 entries, obviously
extern mem_pml4e *i_pmlmax;

extern size_t i_order_ps[ORDER_COUNT];

void vcache_init();

#endif