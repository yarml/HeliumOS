#ifndef HELIUM_ARCH_MEM_H
#define HELIUM_ARCH_MEM_H

#include <attributes.h>
#include <stdint.h>

/* a memory structure pointing to a lower memory structure  */
/* can be a pml4e, pde, pdpte */
struct MEM_VPSTRUCT_PTR;
typedef struct MEM_VPSTRUCT_PTR mem_vpstruct_ptr;
typedef struct MEM_VPSTRUCT_PTR mem_pml4e;
typedef struct MEM_VPSTRUCT_PTR mem_pdpte_ref;
typedef struct MEM_VPSTRUCT_PTR mem_pde_ref;
struct MEM_VPSTRUCT_PTR
{
  uint64_t present :1 ;
  uint64_t write   :1 ;
  uint64_t user    :1 ;
  uint64_t pwt     :1 ;
  uint64_t pcd     :1 ;
  uint64_t accessed:1 ;
  uint64_t free0   :1 ;
  uint64_t ps      :1 ; /* 0 */
  uint64_t free1   :4 ;
  uint64_t ss_padr :36; /* substruct ptr */
  uint64_t res0    :4 ;
  uint64_t free2   :11;
  uint64_t xd      :1 ;
} pack;

/* pdpte, pde with ps=1 */
struct MEM_VPSTRUCT;
typedef struct MEM_VPSTRUCT mem_vpstruct;
typedef struct MEM_VPSTRUCT mem_pdpte_map;
typedef struct MEM_VPSTRUCT mem_pde_map;
struct MEM_VPSTRUCT
{
  uint64_t present :1 ;
  uint64_t write   :1 ;
  uint64_t user    :1 ;
  uint64_t pwt     :1 ;
  uint64_t pcd     :1 ;
  uint64_t accessed:1 ;
  uint64_t dirty   :1 ;
  uint64_t ps      :1 ; /* 1 */
  uint64_t global  :1 ;
  uint64_t free0   :3 ;
  uint64_t pat     :1 ;
  uint64_t padr    :35; /* Address should be aligned to
                           the correct boundary!! */
  uint64_t res0    :4 ; /* 0 */
  uint64_t free1   :7 ;
  uint64_t prot_key:4 ;
  uint64_t xd      :1 ;
} pack;

/* pte struct */
struct MEM_VPSTRUCT2;
typedef struct MEM_VPSTRUCT2 mem_vpstruct2;
typedef struct MEM_VPSTRUCT2 mem_pte;
struct MEM_VPSTRUCT2
{
  uint64_t present :1 ;
  uint64_t write   :1 ;
  uint64_t user    :1 ;
  uint64_t pwt     :1 ;
  uint64_t pcd     :1 ;
  uint64_t accessed:1 ;
  uint64_t dirty   :1 ;
  uint64_t pat     :1 ;
  uint64_t global  :1 ;
  uint64_t free0   :3 ;
  uint64_t padr    :36;
  uint64_t res0    :4 ;
  uint64_t free1   :7 ;
  uint64_t prot_key:4 ;
  uint64_t xd      :1 ;
} pack;

uint16_t mem_vpstruct_ptr_meta(mem_vpstruct_ptr *s);
void mem_vpstruct_ptr_set_meta(mem_vpstruct_ptr *s, uint16_t meta);

uint16_t mem_vpstruct_meta(mem_vpstruct *s);
void mem_vpstruct_set_meta(mem_vpstruct *s, uint16_t meta);

uint16_t mem_vpstruct2_meta(mem_vpstruct2 *s);
void mem_vpstruct2_set_meta(mem_vpstruct2 *s, uint16_t meta);

// Substruct address, s : mem_vpstruct_ptr*
#define SS_PADR(s) (void*) (((s)->ss_padr << 12) & 0xFFFFFFFFFFFFF000)

// Page size of order 0
#define MEM_PS (4096)

#endif