#include <mem.h>

#include <boot_info.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <asm/ctlr.h>
#include <asm/msr.h>
#include <fb.h>
#include <debug.h>

static uint64_t pmm_header      = UINT64_MAX;
static uint64_t mmap_usable_len = 0;

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
        if(MMapEnt_IsFree(c) && MMapEnt_Size(c) >= MEM_PMM_PAGE_SIZE)
        {
#define PG_COUNT (ALIGN_DN(MMapEnt_Size(c), MEM_PMM_PAGE_SIZE) / MEM_PMM_PAGE_SIZE)
            mmap_usable[mmap_usable_len  ].padr     = ALIGN_UP(MMapEnt_Ptr (c), MEM_PMM_PAGE_SIZE);
            mmap_usable[mmap_usable_len++].pg_count = PG_COUNT;

            total_header_size += ALIGN_UP(PG_COUNT / 8, 8);
#undef PG_COUNT
            mem_total += ALIGN_DN(MMapEnt_Size(c), MEM_PMM_PAGE_SIZE);
        }
        else if(MMapEnt_IsFree(c))
            printf("*"); // too small, unused, should be a rare(inexistent?) occurence
        printf("\n");
    }
    printf("Total memory: %lz\n", mem_total);

    // look for a memory region big enough to hold all the bitmaps and headers
    // if it doesnt exist, idc, what type of memory map is this?
    size_t header_pg_count = 
        ALIGN_UP(total_header_size + mmap_usable_len * sizeof(mem_pmm_header), MEM_PMM_PAGE_SIZE) / MEM_PMM_PAGE_SIZE;
    pmm_header = UINT64_MAX;
    for(size_t i = 0; i < mmap_usable_len; ++i)
    {
        if(mmap_usable[i].pg_count >= header_pg_count)
        {
            pmm_header = mmap_usable[i].padr;
            mmap_usable[i].padr     += header_pg_count * MEM_PMM_PAGE_SIZE;
            mmap_usable[i].pg_count -= header_pg_count                    ;
            break;
        }
    }
    if(pmm_header == UINT64_MAX)
    {
        printf("Couldnt find enough space to store the physical memory manager header!\n");
        LOOP;
    }
    void* offset = pmm_header;
    uint64_t empty_entries = 0;
    for(size_t i = 0; i < mmap_usable_len; ++i)
    {
        if(mmap_usable[i].pg_count != 0)
        {
            memcpy(offset, &(mmap_usable[i]), sizeof(mem_pmm_header));
            offset += sizeof(mem_pmm_header);
            memset(offset, 0, MEM_PMM_BITMAP_LEN(mmap_usable[i]));
            offset += MEM_PMM_BITMAP_LEN(mmap_usable[i]);
        }
        else
            ++empty_entries;
    }
    mmap_usable_len -= empty_entries;

/*     ctlr_cr3_npcid cr3 = as_scr3();
    mem_pml4e* pml4 = (mem_pml4e*) CTLR_CR3_NPCID_PML4_PADR(cr3);
    
    printf("size cr3 : %d\n", sizeof(ctlr_cr3_npcid));
    printf("PML4 physical address: %16p\n", pml4);
    

    printf("Listing pml4 present entries\n");
    for(int i = 0; i < 512; ++i)
        if(pml4[i].present)
        {
            printf(" PML4E %3d %16lx\n", i, MEM_PML4E_PDPT_PADR(pml4[i]));

        }
 */
}

// the arg header is only used to be able to alloacte before virtual mapping is done
// this isnt meant to be a flexible function, it is depenedent on global variables
// otherwise use MEM_PMM_ALLOC_PHY_PAGE(count) define when virtual mapping is available
mem_pmm_allocation mem_pmm_alloc_phy_pages(uint64_t header, uint64_t count)
{
    mem_pmm_allocation alloc;
    for(size_t i = 0; i < mmap_usable_len; ++i)
    {

    }
    return alloc;
}
