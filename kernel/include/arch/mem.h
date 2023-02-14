#ifndef HELIUM_ARCH_MEM_H
#define HELIUM_ARCH_MEM_H

#include <stdint.h>

/* a memory structure pointing to a lower memory structure  */
struct MEM_VPSTRUCT_PTR;
typedef struct MEM_VPSTRUCT_PTR mem_vpstruct_ptr;
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

/* pdpte, pdte with ps=1 */
struct MEM_VPSTRUCT;
typedef struct MEM_VPSTRUCT mem_vpstruct;
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

// Substruct address, s : mem_vpstruct_ptr*
#define SS_PADR(s) (void*) (((s)->ss_padr << 12) & 0xFFFFFFFFFFFFF000)

// Page size of order 0
#define MEM_PS (4096)

#endif