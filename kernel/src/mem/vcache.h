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
#define VCACHE_CC_FLUSH   (32) /* How many calls to VCache functions will force
                                  a lazily unmapped page to be unmapped */



struct VCAC_UNIT;
typedef struct VCACHE_UNIT vcache_unit;
struct VCACHE_UNIT
{
    // All the following pointers have the same value, they are there for the
    // convenience of usage
    mem_vpstruct_ptr *vvps_ptr;
    mem_vpstruct *vvps;
    mem_vpstruct2 *vvps2;
    errno_t error;
};

/* Map one page of physical space to virtual memory chosen by vcache system */
vcache_unit vcache_map(void *padr);

/* Lazy unmap, mark the page as to be unmapped if vcache system is called
   multiple times without referencing this page */
void vcache_lumap(vcache_unit unit);
/* Unmap the page, the system can choose to keep it until its space is needed */
void vcache_umap(vcache_unit unit);

/* Unmap all lazily unmapped pages */
void vcache_flush();

#endif