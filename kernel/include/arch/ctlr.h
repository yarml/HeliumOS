#ifndef HELIUM_CTLR_H
#define HELIUM_CTLR_H

// namespace ctlr

#include <stdint.h>

#include <arch/mem.h>

struct CLTR_CR3_NCPID;
typedef struct CLTR_CR3_NCPID ctlr_cr3_npcid;
struct CLTR_CR3_NCPID {
  uint64_t free0     : 3;
  uint64_t pwt       : 1;
  uint64_t pcd       : 1;
  uint64_t free1     : 7;
  uint64_t pml4_padr : 36;
  uint64_t res0      : 16;
} pack;

#define CTLR_CR3_NPCID_PML4_PADR(cr3)                                          \
  ((mem_vpstruct_ptr*)((uint64_t)(cr3).pml4_padr << 12))

#endif