#ifndef HELIUM_MEM_H
#define HELIUM_MEM_H

#include <attributes.h>
#include <stdint.h>
#include <utils.h>

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
    // physical address of the region
    uint64_t padr ;

    // number of physical pages here
    uint64_t pg_count;
} pack mem_pmm_header;
// size should be a multiple of 8

#define MEM_PMM_PAGE_SIZE (4096) // size of pages managed by the physical memory manager

#define MEM_PMM_BITMAP_LEN(h) (ALIGN_UP((h).pg_count / 8, 8))
// header size in pages
#define MEM_PMM_HEADER_SIZE(h) (sizeof(mem_pmm_header) + MEM_PMM_BITMAP_LEN(h))

typedef struct
{
    uint64_t header_off;
    uint64_t page_idx  ;
    uint64_t len       ;
} mem_pmm_allocation;

void mem_init();
// match_type is one of
//  - MEM_PMM_BEST_MATCH
//  - MEM_PMM_FIRST_MATCH

#define MEM_PMM_BEST_MATCH  (0)
#define MEM_PMM_FIRST_MATCH (1)

mem_pmm_allocation mem_pmm_alloc_phy_pages(uint64_t header, uint64_t count, int match_type);

#endif