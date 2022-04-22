#include <mem.h>

#include <boot_info.h>
#include <stddef.h>
#include <stdio.h>
#include <asm/ctlr.h>
#include <asm/msr.h>

void mem_init()
{
    size_t mmap_size = (bootboot.size - sizeof(BOOTBOOT)) / sizeof(MMapEnt) + 1;
    size_t mem_usable  = 0;
    size_t mem_headers = 0;
    size_t mem_total   = 0;
    printf("Memory map entries(%d):\n", mmap_size);
    for(MMapEnt* c = &bootboot.mmap; mmap_size != 0; ++c, --mmap_size)
    {
        printf(
            " - %16p-%16p %10z %4e1:Free,2:ACPI,3:MMIO,#:Used",
            MMapEnt_Ptr(c),
            MMapEnt_Ptr(c) + MMapEnt_Size(c)-1,
            MMapEnt_Size(c), MMapEnt_Type(c)
        );
        if(MMapEnt_IsFree(c) && MMapEnt_Size(c) > 2 * MEM_PMM_PAGE_SIZE)
        {
            printf("*");
            // How the physical memory manager works:
            // 
            
            uint64_t real_start  = MMapEnt_Ptr(c)  / 4096 * 4096; // aligned to 4K
            
            // real size in pages
            uint64_t real_size   = (MMapEnt_Size(c) - (real_start - MMapEnt_Ptr(c))) / 4096; 
            

            // mAtH
            uint64_t util_size = 
                (
                    8 * (real_size * MEM_PMM_PAGE_SIZE - sizeof(mem_pmm_header))
                ) 
                / 
                (
                    8 * MEM_PMM_PAGE_SIZE + 1
                );
            // util_size is the size considering the bitmap and header take space too

            uint64_t header_size = real_size - util_size;

            mem_usable  += util_size   * MEM_PMM_PAGE_SIZE;
            mem_total   += real_size   * MEM_PMM_PAGE_SIZE;
            mem_headers += header_size * MEM_PMM_PAGE_SIZE;
        }
        printf("\n");
    }
    printf(
        " i %16p-%16p %10z\n",
        bootboot.initrd_ptr,
        bootboot.initrd_ptr + bootboot.initrd_size - 1,
        bootboot.initrd_size
    );
    ctlr_cr3_npcid cr3 = as_scr3();
    mem_pml4e* pml4 = (mem_pml4e*) CTLR_CR3_NPCID_PML4_PADR(cr3);
    
    printf("Total memory: %lz\n", mem_total);
    printf("Total header memory: %lz\n", mem_headers);
    printf("Total usable memory: %lz\n", mem_usable);
/*     printf("size cr3 : %d\n", sizeof(ctlr_cr3_npcid));
    printf("PML4 physical address: %16p\n", pml4);
    

    printf("Listing pml4 present entries\n");
    for(int i = 0; i < 512; ++i)
        if(pml4[i].present)
        {
            printf(" PML4E %3d %16lx\n", i, MEM_PML4E_PDPT_PADR(pml4[i]));

        } */

}
