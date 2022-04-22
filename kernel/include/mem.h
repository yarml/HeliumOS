#ifndef HELIUM_MEM_H
#define HELIUM_MEM_H

#include <attributes.h>
#include <stdint.h>

// namespace mem

// x86_64 memory structs
typedef struct
{
    uint64_t present  :1 ;
    uint64_t write    :1 ;
    uint64_t user     :1 ;
    uint64_t pwt      :1 ;
    uint64_t pcd      :1 ;
    uint64_t accessed :1 ;
    uint64_t ig0      :1 ;
    uint64_t zr0      :1 ;
    uint64_t ig1      :4 ;
    uint64_t pdpt_padr:36;
    uint64_t zr1      :4 ;
    uint64_t ig2      :11;
    uint64_t xd       :1 ;
} pack mem_pml4e;

#define MEM_PML4E_PDPT_PADR(e) (e.pdpt_padr << 12)

#define MEM_PML4_LEN (512)
#define MEM_PDPT_LEN (512)
#define MEM_PD_LEN   (512)
#define MEM_PT_LEN   (512)

// HeliumOs memory structs
typedef struct
{
    uint64_t region_phy ;
    uint64_t mem_size   ;
    uint64_t header_size;
    uint64_t free_idx   ;
} mem_pmm_header;

#define MEM_PMM_PAGE_SIZE (4096) // size of pages managed by the physical memory manager

void mem_init();

#endif