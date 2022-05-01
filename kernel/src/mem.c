#include <mem.h>

#include <boot_info.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <asm/ctlr.h>
#include <asm/msr.h>
#include <asm/invlpg.h>
#include <fb.h>
#include <math.h>
#include <debug.h>
#include <asm/scas.h>
#include <errors.h>

static uint64_t pmm_header      = UINT64_MAX;
static uint64_t mmap_usable_len = 0;
static mem_pml4e* pml4;

void mem_init()
{
    size_t mmap_size = (bootboot.size - sizeof(BOOTBOOT)) / sizeof(MMapEnt) + 1;
    size_t mem_total   = 0;

    size_t total_header_size = 0;
    mmap_usable_len = 0;
    mem_pmm_header mmap_usable[mmap_size];

    printf("Memory map entries(%d):\n", mmap_size);
    for(MMapEnt* c = &bootboot.mmap; mmap_size != 0; ++c, --mmap_size)
    {
        printf(
            " - %16p-%16p %10z %4e1:Free,2:ACPI,3:MMIO,#:Used",
            MMapEnt_Ptr(c),
            MMapEnt_Ptr(c) + MMapEnt_Size(c)-1,
            MMapEnt_Size(c), MMapEnt_Type(c)
        );
        if(MMapEnt_IsFree(c) && MMapEnt_Size(c) >= MEM_PAGE_SIZE)
        {
#define PG_COUNT (ALIGN_DN(MMapEnt_Size(c), MEM_PAGE_SIZE) / MEM_PAGE_SIZE)
            mmap_usable[mmap_usable_len  ].padr     = ALIGN_UP(MMapEnt_Ptr (c), MEM_PAGE_SIZE);
            mmap_usable[mmap_usable_len++].pg_count = PG_COUNT;

            total_header_size += ALIGN_UP(PG_COUNT / 8, 8);
#undef PG_COUNT
            mem_total += ALIGN_DN(MMapEnt_Size(c), MEM_PAGE_SIZE);
        }
        else if(MMapEnt_IsFree(c))
            printf("*"); // too small, unused, should be a rare(inexistent?) occurence
        printf("\n");
    }
    printf("Total memory: %lz\n", mem_total);

    // look for a memory region big enough to hold all the bitmaps and headers
    // if it doesnt exist, idc, what type of memory map is this?
    size_t header_pg_count = 
        ALIGN_UP(total_header_size + mmap_usable_len * sizeof(mem_pmm_header), MEM_PAGE_SIZE) / MEM_PAGE_SIZE;
    pmm_header = UINT64_MAX;
    for(size_t i = 0; i < mmap_usable_len; ++i)
    {
        if(mmap_usable[i].pg_count >= header_pg_count)
        {
            pmm_header = mmap_usable[i].padr;
            mmap_usable[i].padr     += header_pg_count * MEM_PAGE_SIZE;
            mmap_usable[i].pg_count -= header_pg_count                    ;
            break;
        }
    }
    if(pmm_header == UINT64_MAX)
    {
        printf("Couldnt find enough space to store the physical memory manager header!\n");
        LOOP;
    }
    void* offset = (void*) pmm_header;
    uint64_t empty_entries = 0;
    for(size_t i = 0; i < mmap_usable_len; ++i)
    {
        if(mmap_usable[i].pg_count != 0)
        {
            memcpy(offset, &(mmap_usable[i]), sizeof(mem_pmm_header));
            offset += sizeof(mem_pmm_header);
            memset(offset, UINT32_MAX, MEM_PMM_BITMAP_LEN(mmap_usable[i]));
            offset += MEM_PMM_BITMAP_LEN(mmap_usable[i]);
        }
        else
            ++empty_entries;
    }
    mmap_usable_len -= empty_entries;

    ctlr_cr3_npcid cr3 = as_scr3();
    pml4 = (mem_pml4e*) CTLR_CR3_NPCID_PML4_PADR(cr3);

    printf("PML4 physical address: %16p\n", pml4);


    mem_vmm_map(pmm_header, 0xFFFF800000000000, pmm_header);
    printf("1\n");
    char* p = (char*) 0xFFFF800000000000;
    *p = 0;
    printf("2\n");
    LOOP;
}

// the arg header is only used to be able to alloacte before virtual mapping is done
// this isnt meant to be a flexible function, it is depenedent on global variables
// otherwise use MEM_PMM_ALLOC_PHY_PAGE(count, match_type) define when virtual mapping is available
mem_pmm_allocation mem_pmm_alloc_phy_pages(uint64_t header, uint64_t count)
{
    mem_pmm_allocation alloc;
    alloc.header_off = UINT64_MAX;
    alloc.len        = 0         ;
    alloc.page_idx   = UINT64_MAX;
    uint64_t org_header = header;

    for(size_t i = 0; i < mmap_usable_len; ++i)
    {
        mem_pmm_header* acheader = (mem_pmm_header*) header;
        uint64_t* bitmap = (uint64_t*) (header + sizeof(mem_pmm_header));
        uint64_t qwords = MEM_PMM_BITMAP_LEN(*acheader) / sizeof(uint64_t);
        
        uint64_t idx = qwords - as_nscasq(header + sizeof(mem_pmm_header), 0, qwords) - 1;
        if(!bitmap[idx]) // all pages allocated in this region
        {
            header += sizeof(mem_pmm_header) + MEM_PMM_BITMAP_LEN(*acheader);
            continue;
        }
        size_t from = FFS(bitmap[idx]);
        size_t to = MIN(MIN(count, 64 - from), FFS((~bitmap[idx]) >> from)) + from;
        bitmap[idx] &= ~(BITRANGE(from, to));
        alloc.header_off = header - org_header;
        alloc.page_idx   = 64 * idx + from;
        alloc.len        = to - from;
        return alloc;
    }
    return alloc;
}

void mem_pmm_dealloc_phy_pages(uint64_t header, mem_pmm_allocation alloc)
{
    uint64_t* bitmap = (uint64_t*) (header + sizeof(mem_pmm_header));
    bitmap[alloc.page_idx / 64] |= BITRANGE(alloc.page_idx % 64, alloc.page_idx % 64 + alloc.len);
}

void* mem_pmm_alloc_adr(uint64_t header, mem_pmm_allocation alloc)
{
    mem_pmm_header* acheader = (mem_pmm_header*) (header + alloc.header_off);
    return (void*) (acheader->padr + alloc.page_idx * MEM_PAGE_SIZE);
}

uint64_t mem_vmm_map(uint64_t pmm_header, uint64_t vadr, uint64_t padr)
{
    if(MEM_PML4_IDX(vadr) == 511)
        error_physical_memory_alloc("Cannot map pages at index 511 of the PML4");

    mem_pml4e* tpml4e = &(pml4[MEM_PML4_IDX(vadr)]);
    if(!tpml4e->present) // we need a new pdpt
    {
        mem_pmm_allocation alloc = mem_pmm_alloc_phy_pages(pmm_header, 1);
        if(!alloc.len)
            error_physical_memory_alloc("Couldn't allocate memory for a new PDPT");
        memset(mem_pmm_alloc_adr(pmm_header, alloc), 0, MEM_PAGE_SIZE);
        tpml4e->write     = 1;
        tpml4e->user      = MEM_PML4_IDX(vadr) < 256;
        tpml4e->pwt       = 0;
        tpml4e->pcd       = 0;
        tpml4e->accessed  = 0;
        tpml4e->ps        = 0;
        tpml4e->pdpt_padr = (uint64_t)(mem_pmm_alloc_adr(pmm_header, alloc)) >> 12;
        tpml4e->zr1       = 0;
        tpml4e->xd        = 0;
        tpml4e->present   = 1;
    }
    mem_pdpte* tpdpte = &(((mem_pdpte*) MEM_PML4E_PDPT_PADR(*tpml4e))[MEM_PDPT_IDX(vadr)]);
    if(!tpdpte->present) // We need a new pd
    {
        mem_pmm_allocation alloc = mem_pmm_alloc_phy_pages(pmm_header, 1);
        if(!alloc.len)
            error_physical_memory_alloc("Couldn't allocate memory for a new PD");
        memset(mem_pmm_alloc_adr(pmm_header, alloc), 0, MEM_PAGE_SIZE);
        tpdpte->write     = 1;
        tpdpte->user      = MEM_PML4_IDX(vadr) < 256;
        tpdpte->pwt       = 0;
        tpdpte->pcd       = 0;
        tpdpte->accessed  = 0;
        tpdpte->ps        = 0;
        tpdpte->pdpt_padr = (uint64_t)(mem_pmm_alloc_adr(pmm_header, alloc)) >> 12;
        tpdpte->zr1       = 0;
        tpdpte->xd        = 0;
        tpdpte->present   = 1;
    }
    mem_pde* tpde = &(((mem_pde*) MEM_PDPT_PD_PADR(*tpdpte))[MEM_PD_IDX(vadr)]);
    if(!tpde->present) // We need a new pt
    {
        mem_pmm_allocation alloc = mem_pmm_alloc_phy_pages(pmm_header, 1);
        if(!alloc.len)
            error_physical_memory_alloc("Couldn't allocate memory for a new PT");
        memset(mem_pmm_alloc_adr(pmm_header, alloc), 0, MEM_PAGE_SIZE);
        tpde->write     = 1;
        tpde->user      = MEM_PML4_IDX(vadr) < 256;
        tpde->pwt       = 0;
        tpde->pcd       = 0;
        tpde->accessed  = 0;
        tpde->ps        = 0;
        tpde->pdpt_padr = (uint64_t)(mem_pmm_alloc_adr(pmm_header, alloc)) >> 12;
        tpde->zr1       = 0;
        tpde->xd        = 0;
        tpde->present   = 1;
    }
    mem_pte* tpte = &(((mem_pte*) MEM_PD_PT_PADR(*tpde))[MEM_PT_IDX(vadr)]);

    tpte->write     = 1;
    tpte->user      = MEM_PML4_IDX(vadr) < 256;
    tpte->pwt       = 0;
    tpte->pcd       = 0;
    tpte->accessed  = 0;
    tpte->dirty     = 0;
    tpte->pat       = 0;
    tpte->g         = MEM_PML4_IDX(vadr) >= 256;
    tpte->phy_padr  = padr >> 12;
    tpte->zr1       = 0;
    tpte->prot_key  = 0;
    tpte->xd        = 0;
    tpte->present   = 1;

    as_invlpg(vadr);
    return vadr;
}

void mem_vmm_unmap(uint64_t vadr)
{
    mem_pml4e* tpml4e = &(pml4[MEM_PML4_IDX(vadr)]);
    if(tpml4e->present)
    {
        mem_pdpte* tpdpte = &(((mem_pdpte*) MEM_PML4E_PDPT_PADR(*tpml4e))[MEM_PDPT_IDX(vadr)]);
        if(tpdpte->present)
        {
            mem_pde* tpde = &(((mem_pde*) MEM_PDPT_PD_PADR(*tpdpte))[MEM_PD_IDX(vadr)]);   
            if(tpde->present)
            {
                mem_pte* tpte = &(((mem_pte*) MEM_PD_PT_PADR(*tpde))[MEM_PT_IDX(vadr)]);
                memset(tpte, 0, sizeof(mem_pte));
            }
        }
    }
    as_invlpg(vadr);
}

