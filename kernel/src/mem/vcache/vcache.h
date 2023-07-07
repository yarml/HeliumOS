#ifndef HELIUM_VCACHE_H
#define HELIUM_VCACHE_H

#include <error.h>
#include <mem.h>

#include "../internal_mem.h"

struct VCACHE_UNIT;
typedef struct VCACHE_UNIT vcache_unit;
struct VCACHE_UNIT {
  void *ptr;
  size_t pde_idx;
  size_t pte_idx;
  errno_t error;
};

#define ERR_VCM_NPG_FOUND (-1)

vcache_unit vcache_map(void *padr);
void vcache_remap(vcache_unit unit, void *padr);

#define VCACHE_NO_ID ((void *)UINTPTR_MAX)
void vcache_umap(vcache_unit unit, void *id);

#define VCACHE_PTR (KVMSPACE)
#define VCACHE_LEN (2048)
#define VCACHE_SIZE (VCACHE_LEN * MEM_PS)

// Index of the PML4E used by the vcache(only one)
#define PML4E_IDX (ENTRY_IDX(3, VCACHE_PTR))
// Index of the PDPTE used by the vcache(only one, parent index is PML4E_IDX)
#define PDPTE_IDX (ENTRY_IDX(2, VCACHE_PTR))

#define PDE_COUNT (VCACHE_LEN / 512)

// This pointer, after memory initialization, should point
// to 4 consecutive PDEs that are used for VCache
extern mem_pde_ref *i_vcache_pde;

// Pointer to the first PTE used by VCache. This PTE should be
// followed by another 2047 PTE
extern mem_pte *i_vcache_pte;

#endif