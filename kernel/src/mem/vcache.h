#ifndef HELIUM_INT_VCACHE_H
#define HELIUM_INT_VCACHE_H

#include <error.h>
#include <mem.h>

#include "internal_mem.h"

struct VCACHE_UNIT;
typedef struct VCACHE_UNIT vcache_unit;
struct VCACHE_UNIT
{
  void *ptr;
  mem_pte *target_pte;
  mem_pde_ref *target_pde;
  errno_t error;
};

#define ERR_VCM_NPG_FOUND (-1)

vcache_unit vcache_map(void *padr);
void vcache_umap(vcache_unit unit);

#define VCACHE_LEN (2048)

#define VCACHE_PTR (KVMSPACE)

// Index of the PML4E used by the vcache(only one)
#define PML4E_IDX (ENTRY_IDX(3, VCACHE_PTR))
// Index of the PDPTE used by the vcache(only one, parent index is PML4E_IDX)
#define PDPTE_IDX (ENTRY_IDX(2, VCACHE_PTR))

#define PDE_COUNT (VCACHE_LEN / 512)

#endif