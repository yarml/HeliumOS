#ifndef HELIUM_INT_MEM_H
#define HELIUM_INT_MEM_H

#include <stddef.h>
#include <math.h>
#include <mem.h>

#include <arch/mem.h>


// TODO: find optimal value
// Threshold, if a virtual mapping needs to modify more than this much pages
// then reloading cr3 is prefered over invlpg (unless the paging is global)
#define RLCR3_THRESHOLD (32)

#define BITMAP_SIZE(seg_size) (ALIGN_UP((seg_size) / MEM_PS, 64) / 8)

#define ADR_MASK(n)  (uintptr_t) ((uintptr_t) 0x1FF << (12 + 9 * (n)))
#define ADR_SHIFT(n) (size_t) (12 + 9 * (n))

#define ENTRY_IDX(n, ptr) \
    (size_t) (((uintptr_t) (ptr) & ADR_MASK(n)) >> ADR_SHIFT(n))

// TODO: when porting to other architectures, this should be considered
#define ORDER_COUNT (4)
#define MAX_ORDER   (ORDER_COUNT - 1)


// TODO: calculate at compile time if possible
#define ORDER_PS(n) (size_t)(MEM_PS * powi(512, (n)))

extern void *i_pmm_header;
extern size_t i_mmap_usable_len;

// The physical address of the VMM structure of order MAX_ORDER(PML4)
// As set by bootboot
extern mem_vpstruct_ptr *i_ppmlmax;

// Virtual address of the VMM structure of MAX_ORDER(PML4)
// After mem_init, this should be also the address of the beginning
// of the VMM super structure
extern mem_vpstruct_ptr *i_pmlmax;

extern size_t i_order_ps[ORDER_COUNT];

static mem_vpstruct_ptr i_default_vpstruct_ptr =
{
  .present  = 0, // present should be set after the address is set
  .write    = 1,
  .user     = 1,
  .pwt      = 0,
  .pcd      = 0,
  .accessed = 0,
  .free0    = 0,
  .ps       = 0,
  .free1    = 0,
  .ss_padr  = 0,
  .res0     = 0,
  .free2    = 0,
  .xd       = 0
};

// defult pde, pdpte with ps = 1
static mem_vpstruct i_default_vpstruct =
{
  .present  = 0, // present should be set after the address is set
  .write    = 0,
  .user     = 0,
  .pwt      = 0,
  .pcd      = 0,
  .accessed = 0,
  .dirty    = 0,
  .ps       = 1,
  .global   = 0,
  .free0    = 0,
  .pat      = 0,
  .padr     = 0,
  .res0     = 0,
  .free1    = 0,
  .prot_key = 0,
  .xd       = 0
};

// default pte
static mem_vpstruct2 i_default_vpstruct2 =
{
  .present  = 0,
  .write    = 0,
  .user     = 0,
  .pwt      = 0,
  .pcd      = 0,
  .accessed = 0,
  .dirty    = 0,
  .pat      = 0,
  .global   = 0,
  .free0    = 0,
  .padr     = 0,
  .res0     = 0,
  .free1    = 0,
  .prot_key = 0,
  .xd       = 0
};

#endif