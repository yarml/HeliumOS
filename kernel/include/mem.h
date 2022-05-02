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
    uint64_t present :1 ;
    uint64_t write   :1 ;
    uint64_t user    :1 ;
    uint64_t pwt     :1 ;
    uint64_t pcd     :1 ;
    uint64_t accessed:1 ;
    uint64_t ig0     :1 ;
    uint64_t ps      :1 ;
    uint64_t ig1     :4 ;
    uint64_t pd_padr :36;
    uint64_t zr1     :4 ;
    uint64_t ig2     :11;
    uint64_t xd      :1 ;
} pack mem_pdpte;


typedef struct
{
    uint64_t present :1 ;
    uint64_t write   :1 ;
    uint64_t user    :1 ;
    uint64_t pwt     :1 ;
    uint64_t pcd     :1 ;
    uint64_t accessed:1 ;
    uint64_t ig0     :1 ;
    uint64_t ps      :1 ;
    uint64_t ig1     :4 ;
    uint64_t pt_padr :36;
    uint64_t zr1     :4 ;
    uint64_t ig2     :11;
    uint64_t xd      :1 ;
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

#define MEM_PML4_PDPT_PADR(e) ((uint64_t)((uint64_t)(e).pdpt_padr << 12))
#define MEM_PDPT_PD_PADR(  e) ((uint64_t)((uint64_t)(e).pd_padr   << 12))
#define MEM_PD_PT_PADR(    e) ((uint64_t)((uint64_t)(e).pt_padr   << 12))
#define MEM_PT_PADR(       e) ((uint64_t)((uint64_t)(e).phy_padr  << 12))

#define MEM_PML4_IDX(pg) (((0x0000FF8000000000 & pg) >> 39) % MEM_PML4_LEN)
#define MEM_PDPT_IDX(pg) (((0x0000FFFFC0000000 & pg) >> 30) % MEM_PDPT_LEN)
#define MEM_PD_IDX(  pg) (((0x0000FFFFFFE00000 & pg) >> 21) % MEM_PD_LEN  )
#define MEM_PT_IDX(  pg) (((0x0000FFFFFFFFFF00 & pg) >> 12) % MEM_PT_LEN  )
#define MEM_ABS_IDX( pg) (((0x0000FFFFFFFFFF00 & pg) >> 12)               )

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
    uint64_t len       ; // in pages
} mem_pmm_allocation;

void mem_init();

mem_pml4e* mem_pml4();

mem_pmm_allocation mem_pmm_alloc_phy_pages  (uint64_t header, uint64_t                   count);
void               mem_pmm_dealloc_phy_pages(uint64_t header, mem_pmm_allocation         alloc);
void               mem_pmm_dealloc_ptr      (uint64_t header, uint64_t phy_ptr, uint64_t len);

void* mem_pmm_alloc_adr(uint64_t header, mem_pmm_allocation alloc);

uint64_t mem_vmm_map  (uint64_t pmm_header_, uint64_t vadr, uint64_t padr, uint64_t size);
uint64_t mem_vmm_unmap(uint64_t pmm_header_, uint64_t vadr, uint64_t size);

// Finds len free pages in virtual space after page start
uint64_t mem_vmm_alloc_range(uint64_t start, uint64_t max, uint64_t len);

#define MEM_PPG_ALLOC(count)          (mem_pmm_alloc_phy_pages(VIRT_PMM_HEADER, (count)))
#define MEM_PPG_DEALLOC(alloc)        (mem_pmm_dealloc_phy_pages(VIRT_PMM_HEADER, (alloc)))
#define MEM_PPTR_DEALLOC(pptr, count) (mem_pmm_dealloc_ptr(VIRT_PMM_HEADER, (pptr), (count)))
#define MEM_MAP(vadr, padr, size)     (mem_vmm_map(VIRT_PMM_HEADER, (vadr), (padr), (size)))
#define MEM_UNMAP(vadr)               (mem_vmm_unmap(VIRT_PMM_HEADER, (vadr)))

#define MEM_PREFER_RLCR3_AT (64) // How many pages do we need to change to prefere rl_cr3 over invlpg, note: invlpg is always used for kernel space pages

// namespace virt
// contains virtual addresses of kernel structures

#define VIRT_PMM_HEADER (0xFFFF800000000000)

#define VIRT_HEAP       (0xFFFF808000000000)

#endif