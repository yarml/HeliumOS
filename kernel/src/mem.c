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

mem_pml4e* mem_pml4()
{
    return pml4;
}

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

    printf("PMM header physical address: %16p\n", pmm_header);

    // Setup virtual memory

    ctlr_cr3_npcid cr3 = as_scr3();
    pml4 = (mem_pml4e*) CTLR_CR3_NPCID_PML4_PADR(cr3);

    // Map the first 512 Gib to VIRT_PHY_OFF
    // That means mapping the first PDPT pointed to by pml4[MEM_PML4_IDX(VIRT_PHY_OFF)]
    // To optimize space, we'll set the PS flag for the PDPTEs to map 1Gib at a time
    
    // The target pml4e
    mem_pml4e* tpml4e = &(pml4[MEM_PML4_IDX(VIRT_PHY_OFF)]);
    // Allocate space for the PDPT
    mem_pmm_allocation alloc = mem_pmm_alloc_phy_pages(pmm_header, 1);
    if(!alloc.len)
        error_physical_memory_alloc("Couldn't allocate PDPT for VIRT_PHY_OFF");
    void* alloc_adr = mem_pmm_alloc_adr(pmm_header, alloc);
    memset(alloc_adr, 0, MEM_PAGE_SIZE);
    tpml4e->write     = 1;
    tpml4e->user      = 0;
    tpml4e->pwt       = 0;
    tpml4e->pcd       = 0;
    tpml4e->accessed  = 0;
    tpml4e->ps        = 0;
    tpml4e->pdpt_padr = ((uint64_t)(alloc_adr)) >> 12;
    tpml4e->zr1       = 0;
    tpml4e->xd        = 0;
    tpml4e->present   = 1;
    for(size_t i = 0; i < 512; ++i)
    {
        mem_pdpte_ps* pdpt = ((mem_pdpte_ps*)(MEM_PML4_PDPT_PADR(*tpml4e))) + i;
        pdpt->write    = 1;
        pdpt->user     = 0;
        pdpt->pwt      = 0;
        pdpt->pcd      = 0;
        pdpt->accessed = 0;
        pdpt->dirty    = 0;
        pdpt->ps       = 1;
        pdpt->g        = 1;
        pdpt->pat      = 0;
        pdpt->zr0      = 0;
        pdpt->phy_padr = i;
        pdpt->zr1      = 0;
        pdpt->prot_key = 0;
        pdpt->xd       = 0;
        pdpt->present  = 1;
    }
    as_rlcr3();
    printf("Done VIRT_PHY_OFF mapping\n");
    char* ptr = 0;
    char* vptr = TO_VIRTUAL(ptr);
    if(*ptr != *vptr)
        printf("not grape\n");
    else
        printf("grape\n");
    printf("Mapping PMM header\n");
    // Map pmm_header to MEM_VIRT_PMM_HEADER
    mem_vmm_map(pmm_header, VIRT_PMM_HEADER, pmm_header, header_pg_count * MEM_PAGE_SIZE);
    printf("Mapped PMM header\n");
}

void mem_rm_identity_map()
{
    memset(pml4, 0, 16 * sizeof(mem_pml4e));
    as_rlcr3();
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

// phy_ptr in bytes
// len in pages
void mem_pmm_dealloc_ptr(uint64_t header, uint64_t phy_ptr, uint64_t len)
{
    mem_pmm_header* acheader = (mem_pmm_header*) header;
    while(!(acheader->padr < phy_ptr && phy_ptr < acheader->padr + acheader->pg_count * MEM_PAGE_SIZE))
        acheader = (mem_pmm_header*) ((uint64_t) acheader + sizeof(mem_pmm_header) + MEM_PMM_BITMAP_LEN(*acheader));
    // if the input address was nowhere in the physical address space
    if(!(acheader->padr < phy_ptr && phy_ptr < acheader->padr + acheader->pg_count * MEM_PAGE_SIZE))
        return;
    mem_pmm_allocation alloc;
    alloc.header_off = (uint64_t) acheader - header;
    alloc.page_idx   = (phy_ptr - acheader->padr) / MEM_PAGE_SIZE;
    alloc.len = len;
    mem_pmm_dealloc_phy_pages(header, alloc);
}

void* mem_pmm_alloc_adr(uint64_t header, mem_pmm_allocation alloc)
{
    mem_pmm_header* acheader = (mem_pmm_header*) (header + alloc.header_off);
    return TO_VIRTUAL((void*) (acheader->padr + alloc.page_idx * MEM_PAGE_SIZE));
}

static void init_page(uint64_t pmm_header_, void* pgstruct, uint64_t vadr)
{
    // All structs have the same structure as the pml4e(except the PTE) so we just use that
    // same for the next structure's address, it is at the same offset of 12
    mem_pml4e* tpgstruct = pgstruct;
    if(!tpgstruct->present) // we need a new structure
    {
        mem_pmm_allocation alloc = mem_pmm_alloc_phy_pages(pmm_header_, 1);
        if(!alloc.len)
            error_physical_memory_alloc("Couldn't allocate memory for a new paging structure");
        void* alloc_adr = mem_pmm_alloc_adr(pmm_header_, alloc);
        memset(alloc_adr, 0, MEM_PAGE_SIZE);
        tpgstruct->write     = 1;
        tpgstruct->user      = MEM_PML4_IDX(vadr) < 256;
        tpgstruct->pwt       = 0;
        tpgstruct->pcd       = 0;
        tpgstruct->accessed  = 0;
        tpgstruct->ps        = 0;
        tpgstruct->pdpt_padr = ((uint64_t)(alloc_adr)) >> 12;
        tpgstruct->zr1       = 0;
        tpgstruct->xd        = 0;
        tpgstruct->present   = 1;
    }
}

// vadr in bytes
// padr in bytes
// size in bytes
uint64_t mem_vmm_map(uint64_t pmm_header_, uint64_t vadr, uint64_t padr, uint64_t size)
{
    size = ALIGN_UP(size, MEM_PAGE_SIZE) / MEM_PAGE_SIZE;

    for(size_t i = 0; i < size; ++i)
    {
        if(MEM_PML4_IDX(vadr) >= 510) // The no touch zone
            error_physical_memory_alloc("Cannot map pages at index 510/511 of the PML4");
        mem_pml4e* tpml4e = TO_VIRTUAL(&(pml4[MEM_PML4_IDX(vadr)]));
        init_page(pmm_header_, tpml4e, vadr);
        mem_pdpte* tpdpte = TO_VIRTUAL(&(((mem_pdpte*) MEM_PML4_PDPT_PADR(*tpml4e))[MEM_PDPT_IDX(vadr)]));
        init_page(pmm_header_, tpdpte, vadr);
        mem_pde* tpde = TO_VIRTUAL(&(((mem_pde*) MEM_PDPT_PD_PADR(*tpdpte))[MEM_PD_IDX(vadr)]));
        init_page(pmm_header_, tpde, vadr);
        mem_pte* tpte = TO_VIRTUAL(&(((mem_pte*) MEM_PD_PT_PADR(*tpde))[MEM_PT_IDX(vadr)]));

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

        
        if(size < MEM_PREFER_RLCR3_AT || MEM_PML4_IDX(vadr) >= 256)
            as_invlpg(vadr);
        vadr += MEM_PAGE_SIZE;
        padr += MEM_PAGE_SIZE;
    }
    if(size >= MEM_PREFER_RLCR3_AT && MEM_PML4_IDX(vadr) < 256)
        as_rlcr3();
    return vadr;
}

// FIXME: idk if this works, I really wrote it without thinking
uint64_t mem_vmm_unmap(uint64_t pmm_header_, uint64_t vadr, uint64_t size)
{
    uint64_t phy_ptr = UINT64_MAX;
    for(size_t i = 0; i < size; ++i)
    {
        mem_pml4e* tpml4e = &(pml4[MEM_PML4_IDX(vadr)]);
        if(tpml4e->present)
        {
            mem_pdpte* tpdpte = &(((mem_pdpte*) MEM_PML4_PDPT_PADR(*tpml4e))[MEM_PDPT_IDX(vadr)]);
            if(tpdpte->present)
            {
                mem_pde* tpde = &(((mem_pde*) MEM_PDPT_PD_PADR(*tpdpte))[MEM_PD_IDX(vadr)]);   
                if(tpde->present)
                {
                    mem_pte* tpte = &(((mem_pte*) MEM_PD_PT_PADR(*tpde))[MEM_PT_IDX(vadr)]);
                    if(i == 0)
                        phy_ptr = MEM_PT_PADR(*tpte);

                    *((uint64_t*)tpte) = 0;
                    if(size < MEM_PREFER_RLCR3_AT || MEM_PML4_IDX(vadr) >= 256)
                        as_invlpg(vadr);
                    vadr += MEM_PAGE_SIZE;
                    // If all PTEs are clear, then this PT isn't needed anymore
                    if(!memnchr((void*) MEM_PD_PT_PADR(*tpde), 0, sizeof(mem_pte) * MEM_PT_LEN))
                    {
                        mem_pmm_dealloc_ptr(pmm_header_, MEM_PD_PT_PADR(*tpde), 1);
                        *((uint64_t*)tpde) = 0;
                    }
                    else
                        continue;
                }
                // If all PDEs are clear, then this PD isn't needed anymore
                if(!memnchr((void*) MEM_PDPT_PD_PADR(*tpdpte), 0, sizeof(mem_pde) * MEM_PD_LEN))
                {
                    mem_pmm_dealloc_ptr(pmm_header_, MEM_PDPT_PD_PADR(*tpdpte), 1);
                    *((uint64_t*)tpdpte) = 0;
                }
                else
                    continue;
            }
            // If all PDPTEs are clear, then this PDPT isn't needed anymore
            if(!memnchr((void*) MEM_PML4_PDPT_PADR(*tpml4e), 0, sizeof(mem_pdpte) * MEM_PDPT_LEN))
            {
                mem_pmm_dealloc_ptr(pmm_header_, MEM_PML4_PDPT_PADR(*tpml4e), 1);
                *((uint64_t*)tpml4e) = 0;
            }
            else
                continue;
        }
    }
    if(size >= MEM_PREFER_RLCR3_AT && MEM_PML4_IDX(vadr) < 256)
        as_rlcr3();
    return phy_ptr;
}

// start in pages
// max in pages
// len in pages
// result in pages
uint64_t mem_vmm_alloc_range(uint64_t start, uint64_t max, uint64_t len)
{
    printf("Allocating range starting from%lx\n");
    size_t first = UINT64_MAX;
    size_t size = 0;
    for(size_t i = start; i < start + max; ++i)
    {
        mem_pml4e* tpml4e = &(pml4[MEM_PML4_IDX(i * MEM_PAGE_SIZE)]);
        if(tpml4e->present)
        {
            mem_pdpte* tpdpte = &(((mem_pdpte*) MEM_PML4_PDPT_PADR(*tpml4e))[MEM_PDPT_IDX(i * MEM_PAGE_SIZE)]);
            if(tpdpte->present)
            {
                mem_pde* tpde = &(((mem_pde*) MEM_PDPT_PD_PADR(*tpdpte))[MEM_PD_IDX(i * MEM_PAGE_SIZE)]);   
                if(tpde->present)
                {
                    mem_pte* tpte = &(((mem_pte*) MEM_PD_PT_PADR(*tpde))[MEM_PT_IDX(i * MEM_PAGE_SIZE)]);
                    if(tpte->present && size != 0)
                    {
                        printf("F", first, i, tpte, *(uint64_t*) tpte);
                        first = UINT64_MAX;
                        size = 0;   
                    }
                    else
                    {
                        if(first == UINT64_MAX)
                            first = i;
                        ++size;
                    }
                }
                else
                {
                    if(first == UINT64_MAX)
                        first = i;
                    i += 512 - 1;
                    size += 512;
                }
            }
            else
            {
                if(first == UINT64_MAX)
                    first = i;
                i += 512*512 - 1;
                size += 512*512;
            }
        }
        else
        {
            if(first == UINT64_MAX)
                first = i;
            i += 512*512*512 - 1;
            size += 512*512*512;
        }
        //printf("good 11 %lu\n", len);
        if(size >= len)
        {
            uint64_t talloc = 0;
            while(len - talloc)
            {
                //printf("Remaining: %lu\n", len - talloc);
                mem_pmm_allocation palloc = mem_pmm_alloc_phy_pages(VIRT_PMM_HEADER, len - talloc);
                if(palloc.len)
                {
                    mem_vmm_map(
                        VIRT_PMM_HEADER,
                        (first + talloc) * MEM_PAGE_SIZE,
                        (uint64_t) mem_pmm_alloc_adr(VIRT_PMM_HEADER, palloc),
                        palloc.len * MEM_PAGE_SIZE
                    );
                    talloc += palloc.len;
                    //printf("Allocated %lu Remaining %lu\n", palloc.len, len - talloc);
                }
                else // not enough physical memory
                {
                    for(size_t k = first; k < first + talloc; ++k)
                    {
                        uint64_t padr = mem_vmm_unmap(VIRT_PMM_HEADER, k * MEM_PAGE_SIZE, MEM_PAGE_SIZE);
                        mem_pmm_dealloc_ptr(VIRT_PMM_HEADER, padr, 1);
                    }
                    return 0;
                }
            }
            printf("Settled on %lx\n", first);
            return first;
        }
    }
    return 0;
}
