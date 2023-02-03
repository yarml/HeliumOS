#ifndef HELIUM_INT_VCACHE_H
#define HELIUM_INT_VCACHE_H

#include <error.h>
#include <mem.h>

// should be divisible by 512G
#define VCACHE_PTR        ((void*)(KVMSPACE))

#define VCACHE_PAGES      (512) /* Should be a multiple of 64 for x86_64, other
                                   architectures: multiple of 8 * sizeof(size_t)
                                */
#define VCACHE_LAZY_PAGES (16)

struct VCAC_UNIT;
typedef struct VCACHE_UNIT vcache_unit;
struct VCACHE_UNIT
{
    void* ptr;
    errno_t error;
};


#define ERR_VCM_NPG_FOUND (-1)

/* Map one page of physical space to virtual memory chosen by vcache system */
vcache_unit vcache_map(void *padr);

/* Unmap the page, the system can choose to keep it until its space is needed
   WARNING: This function should be called once for each vcache unit
            The behaviour is undefined if it is called on a free page */
void vcache_umap(vcache_unit unit);

/* Unmap all lazily unmapped pages */
void vcache_flush();

#define VCP_BITMAP_LEN (VCACHE_PAGES * 2 / (8 * sizeof(size_t)))

extern size_t i_vcp_bitmap[VCP_BITMAP_LEN];

// Indices of lazily unmapped pages
extern size_t i_vclp_table[VCACHE_LAZY_PAGES];

// Count of lazily unmapped pages
extern size_t i_vcache_lazy_count;

#endif