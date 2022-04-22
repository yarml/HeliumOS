#ifndef HELIUM_CTLR_H
#define HELIUM_CTLR_H

// namespace ctlr

#include <stdbool.h>

typedef struct
{
    uint64_t  ig0     :3 ;
    uint64_t  pwt     :1 ;
    uint64_t  pcd     :1 ;
    uint64_t ig1      :7 ;
    uint64_t pml4_padr:36;
    uint64_t zr0      :16;
} pack ctlr_cr3_npcid;

#define CTLR_CR3_NPCID_PML4_PADR(cr3) ((uint64_t)(cr3.pml4_padr << 12))

#endif