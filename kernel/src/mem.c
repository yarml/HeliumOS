#include <boot_info.h>
#include <errors.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>
#include <mem.h>

#define BITMAP_SIZE(index) (ALIGN_UP(ALIGN_DN(MMapEnt_Size(mmap + (index)), MEM_PS) / MEM_PS, 64) / 8)

void* pmm_header = 0;

void mem_init()
{
    printf("Initializing memory\n");

    size_t mmap_len = (bootboot.size - sizeof(BOOTBOOT)) / sizeof(MMapEnt) + 1;

    size_t mmap_usable_len = 0;
    mem_pmm_region_header mmap_usable[mmap_len];

    size_t pmm_header_total_size = 0;

    MMapEnt* mmap = &(bootboot.mmap);

    // Insert the regions backward, so that the PMM would prefer allocating pages
    // with higher physical adresses and leave the lower addresses for hardware that requires it
    for(size_t i = mmap_len; i != 0; --i)
    {
        if(MMapEnt_IsFree(mmap + i - 1) && MMapEnt_Size(mmap + i - 1) >= MEM_PS)
        {
            mmap_usable[mmap_usable_len  ].padr = (void*) ALIGN_UP(MMapEnt_Ptr(mmap + i - 1), MEM_PS);
            mmap_usable[mmap_usable_len++].size = ALIGN_DN(MMapEnt_Size(mmap + i - 1), MEM_PS);
            pmm_header_total_size += BITMAP_SIZE(i - 1) + sizeof(mem_pmm_region_header);
        }
    }

    // Search for a place for the PMM header
    for(size_t i = 0; i < mmap_usable_len; ++i)
    {
        if(mmap_usable[i].size >= ALIGN_UP(pmm_header_total_size, MEM_PS))
        {
            pmm_header = mmap_usable[i].padr;
            mmap_usable[i].padr += ALIGN_UP(pmm_header_total_size, MEM_PS);
            mmap_usable[i].size -= ALIGN_UP(pmm_header_total_size, MEM_PS);
            break;
        }
    }

    // Initialize the PMM header
    size_t pmm_header_offset = 0;
    for(size_t i = 0; i < mmap_usable_len; ++i)
    {
        memcpy(pmm_header + pmm_header_offset, mmap_usable + i, sizeof(mem_pmm_region_header));
        memset(pmm_header + pmm_header_offset + sizeof(mem_pmm_region_header), 0, BITMAP_SIZE(i));
        pmm_header_offset += BITMAP_SIZE(i) + sizeof(mem_pmm_region_header);
    }
}
