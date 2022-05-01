#ifndef HELIUM_MEM_H
#define HELIUM_MEM_H

#include <attributes.h>
#include <stdint.h>
#include <utils.h>

// namespace mem
#define MEM_PML4_LEN (512)
#define MEM_PDPT_LEN (512)
#define MEM_PD_LEN   (512)
#define MEM_PT_LEN   (512)

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
    uint64_t ps       :1 ; // = 0
    uint64_t ig1      :4 ;
    uint64_t pdpt_padr:36;
    uint64_t zr1      :4 ;
    uint64_t ig2      :11;
    uint64_t xd       :1 ;
} pack mem_pml4e;

typedef struct
{
    uint64_t present  :1 ;
    uint64_t write    :1 ;
    uint64_t user     :1 ;
    uint64_t pwt      :1 ;
    uint64_t pcd      :1 ;
    uint64_t accessed :1 ;
    uint64_t ig0      :1 ;
    uint64_t ps       :1 ;
    uint64_t ig1      :4 ;
    uint64_t pdpt_padr:36;
    uint64_t zr1      :4 ;
    uint64_t ig2      :11;
    uint64_t xd       :1 ;
} pack mem_pdpte;


typedef struct
{
    uint64_t present  :1 ;
    uint64_t write    :1 ;
    uint64_t user     :1 ;
    uint64_t pwt      :1 ;
    uint64_t pcd      :1 ;
    uint64_t accessed :1 ;
    uint64_t ig0      :1 ;
    uint64_t ps       :1 ;
    uint64_t ig1      :4 ;
    uint64_t pdpt_padr:36;
    uint64_t zr1      :4 ;
    uint64_t ig2      :11;
    uint64_t xd       :1 ;
} pack mem_pde;

typedef struct
{
    uint64_t present :1 ;
    uint64_t write   :1 ;
    uint64_t user    :1 ;
    uint64_t pwt     :1 ;
    uint64_t pcd     :1 ;
    uint64_t accessed:1 ;
    uint64_t dirty   :1 ;
    uint64_t pat     :1 ;
    uint64_t g       :1 ;
    uint64_t ig0     :3 ;
    uint64_t phy_padr:36;
    uint64_t zr1     :4 ;
    uint64_t ig2     :7 ;
    uint64_t prot_key:4 ;
    uint64_t xd      :1 ;
} pack mem_pte;

#define MEM_PML4E_PDPT_PADR(e) (uint64_t)((uint64_t)(e).pdpt_padr << 12)
#define MEM_PDPT_PD_PADR(   e) (uint64_t)((uint64_t)(e).pdpt_padr << 12)
#define MEM_PD_PT_PADR(     e) (uint64_t)((uint64_t)(e).pdpt_padr << 12)
#define MEM_PT_PADR(        e) (uint64_t)((uint64_t)(e).pdpt_padr << 12)

#define MEM_PML4_IDX(pg) (((pg) / (512 * 1024UL*1024UL*1024UL/MEM_PAGE_SIZE)) % MEM_PML4_LEN)
#define MEM_PDPT_IDX(pg) (((pg) / (1   * 1024UL*1024UL*1024UL/MEM_PAGE_SIZE)) % MEM_PDPT_LEN)
#define MEM_PD_IDX(  pg) (((pg) / (2   * 1024*1024           /MEM_PAGE_SIZE)) % MEM_PD_LEN  )
#define MEM_PT_IDX(  pg) (((pg) / (4   * 1024                /MEM_PAGE_SIZE)) % MEM_PT_LEN  )

// HeliumOs memory structs
typedef struct
{
    // physical address of the region
    uint64_t padr ;

    // number of physical pages here
    uint64_t pg_count;
} pack mem_pmm_header;
// size should be a multiple of 8

#define MEM_PAGE_SIZE (4096) // size of pages managed by the physical memory manager

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

mem_pmm_allocation mem_pmm_alloc_phy_pages  (uint64_t header, uint64_t           count);
void               mem_pmm_dealloc_phy_pages(uint64_t header, mem_pmm_allocation alloc);

#endif