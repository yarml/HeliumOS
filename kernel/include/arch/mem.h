#ifndef HELIUM_ARCH_MEM_H
#define HELIUM_ARCH_MEM_H

#include <attributes.h>
#include <stdint.h>

// Segmentation structures
typedef struct GDTR      gdtr;
typedef struct GDT_ENTRY gdt_entry;

struct GDTR {
  uint16_t   limit;
  gdt_entry *base;
} pack;

struct GDT_ENTRY {
  uint64_t limit0      : 16;
  uint64_t base0       : 24;
  uint64_t accessed    : 1;
  uint64_t write       : 1;
  uint64_t direction   : 1;
  uint64_t exec        : 1;
  uint64_t nsys        : 1;  // Must be 1
  uint64_t dpl         : 2;
  uint64_t present     : 1;
  uint64_t limit1      : 4;
  uint64_t res0        : 1;
  uint64_t lmode       : 1;
  uint64_t size        : 1;
  uint64_t granularity : 1;
  uint64_t base1       : 8;
} pack;

typedef struct TSS_ENTRY {
  uint64_t limit0      : 16;
  uint64_t base0       : 24;
  uint64_t type        : 4; // Must be 0b1001
  uint64_t nsys        : 1;  // Must be 0
  uint64_t dpl         : 2;
  uint64_t present     : 1;
  uint64_t limit1      : 4;
  uint64_t res0        : 3;
  uint64_t granularity : 1;
  uint64_t base1       : 40;
  uint64_t res1        : 32;
} pack tss_entry;

typedef struct TSS {
  uint32_t res0;
  uint64_t rsp[3];
  uint64_t res1;
  uint64_t ist[7];
  uint64_t res2;
  uint16_t res3;
  uint16_t iomap_base;
} pack tss;

// Paging structures

/* a memory structure pointing to a lower memory structure  */
/* can be a pml4e, pde, pdpte, with ps = 0 */
struct MEM_VPSTRUCT_PTR;
typedef struct MEM_VPSTRUCT_PTR mem_vpstruct_ptr;
typedef struct MEM_VPSTRUCT_PTR mem_pml4e;
typedef struct MEM_VPSTRUCT_PTR mem_pdpte_ref;
typedef struct MEM_VPSTRUCT_PTR mem_pde_ref;
struct MEM_VPSTRUCT_PTR {
  uint64_t present  : 1;
  uint64_t write    : 1;
  uint64_t user     : 1;
  uint64_t pwt      : 1;
  uint64_t pcd      : 1;
  uint64_t accessed : 1;
  uint64_t free0    : 1;
  uint64_t ps       : 1; /* 0 */
  uint64_t free1    : 4;
  uint64_t ss_padr  : 36; /* substruct ptr */
  uint64_t res0     : 4;
  uint64_t free2    : 11;
  uint64_t xd       : 1;
} pack;

/* pdpte, pde with ps=1 */
struct MEM_VPSTRUCT;
typedef struct MEM_VPSTRUCT mem_vpstruct;
typedef struct MEM_VPSTRUCT mem_pdpte_map;
typedef struct MEM_VPSTRUCT mem_pde_map;
struct MEM_VPSTRUCT {
  uint64_t present  : 1;
  uint64_t write    : 1;
  uint64_t user     : 1;
  uint64_t pwt      : 1;
  uint64_t pcd      : 1;
  uint64_t accessed : 1;
  uint64_t dirty    : 1;
  uint64_t ps       : 1; /* 1 */
  uint64_t global   : 1;
  uint64_t free0    : 3;
  uint64_t pat      : 1;
  uint64_t padr     : 35; /* Address should be aligned to
                             the correct boundary!! */
  uint64_t res0     : 4;  /* 0 */
  uint64_t free1    : 7;
  uint64_t prot_key : 4;
  uint64_t xd       : 1;
} pack;

/* pte struct */
struct MEM_VPSTRUCT2;
typedef struct MEM_VPSTRUCT2 mem_vpstruct2;
typedef struct MEM_VPSTRUCT2 mem_pte;
struct MEM_VPSTRUCT2 {
  uint64_t present  : 1;
  uint64_t write    : 1;
  uint64_t user     : 1;
  uint64_t pwt      : 1;
  uint64_t pcd      : 1;
  uint64_t accessed : 1;
  uint64_t dirty    : 1;
  uint64_t pat      : 1;
  uint64_t global   : 1;
  uint64_t free0    : 3;
  uint64_t padr     : 36;
  uint64_t res0     : 4;
  uint64_t free1    : 7;
  uint64_t prot_key : 4;
  uint64_t xd       : 1;
} pack;

uint16_t mem_vpstruct_ptr_meta(mem_vpstruct_ptr *s);
void     mem_vpstruct_ptr_set_meta(mem_vpstruct_ptr *s, uint16_t meta);

uint16_t mem_vpstruct_meta(mem_vpstruct *s);
void     mem_vpstruct_set_meta(mem_vpstruct *s, uint16_t meta);

uint16_t mem_vpstruct2_meta(mem_vpstruct2 *s);
void     mem_vpstruct2_set_meta(mem_vpstruct2 *s, uint16_t meta);

// Substruct address, s : mem_vpstruct_ptr*
#define SS_PADR(s) (void *)(((s)->ss_padr << 12) & 0xFFFFFFFFFFFFF000)

// Page size of order 0
#define MEM_PS (4096)

#endif