#include <boot_info.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>
#include <mem.h>

#include <asm/scas.h>

#define BITMAP_SIZE(seg_size) (ALIGN_UP((seg_size) / MEM_PS, 64) / 8)

void* pmm_header = 0;
size_t mmap_usable_len = 0;


void mem_init()
{
    printf("begin mem_init()\n");

    size_t mmap_len = (bootboot.size - sizeof(BOOTBOOT)) / sizeof(MMapEnt) + 1;

    mem_pseg_header mmap_usable[mmap_len];

    size_t pmm_header_total_size = 0;

    MMapEnt* mmap = &(bootboot.mmap);

    /* Insert the regions backward, so that the PMM would prefer allocating pages
       with higher physical adresses and leave the lower addresses for hardware that requires it */
    for(size_t i = mmap_len; i != 0; --i)
    {
        if(MMapEnt_IsFree(mmap + i - 1) && MMapEnt_Size(mmap + i - 1) >= MEM_PS)
        {
            mmap_usable[mmap_usable_len  ].padr = (void*) ALIGN_UP(MMapEnt_Ptr(mmap + i - 1), MEM_PS);
            mmap_usable[mmap_usable_len++].size = ALIGN_DN(MMapEnt_Size(mmap + i - 1), MEM_PS);
            pmm_header_total_size += BITMAP_SIZE(MMapEnt_Size(mmap + i - 1)) + sizeof(mem_pseg_header);
        }
    }

    /* Search for a place for the PMM header */
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

    /* Initialize the PMM header */
    size_t pmm_header_off = 0;
    for(size_t i = 0; i < mmap_usable_len; ++i)
    {
        mem_pseg_header* h = pmm_header + pmm_header_off;
        *h = *(mmap_usable + i);
        memset((void*) h + sizeof(mem_pseg_header), 0, BITMAP_SIZE(h->size));
        /* Mark the leftover pages from the bitmap as used */
        uint64_t* last = (void*) h + sizeof(mem_pseg_header) + BITMAP_SIZE(h->size) - 8;
        *last = UINT64_MAX << (h->size / MEM_PS) % 64;
        pmm_header_off += BITMAP_SIZE(mmap_usable[i].size) + sizeof(mem_pseg_header);
        printf("header(%016p,%016p,%05lu)\n", h, h->padr, h->size / MEM_PS);
    }
    printf("pmm header(%016p, %05lu)\n", pmm_header, pmm_header_off);

    printf("end mem_init()\n");
}

mem_pallocation mem_ppalloc(void* pheader, size_t size, bool cont, void* below)
{
    printf("begin mem_ppaloc(%016p,%lu,%d,%016p)\n", pheader, size, cont, below);
    mem_pallocation alloc;
    alloc.header = 0;
    alloc.padr = 0;
    alloc.size = 0;

    size_t pmm_header_off = 0;
    for(size_t i = 0; i < mmap_usable_len; ++i)
    {
        mem_pseg_header* h = pheader + pmm_header_off;
        size_t bitmap_size = BITMAP_SIZE(h->size);
        printf("checking header %016p\n", h);

        if((!cont || h->size >= size)   /* big enough, without considering allocated pages */
        && (!below || h->padr < below)) /* lowest address low enough */
        {
            printf("header meets minimum criteria\n");
            uint64_t* bitmap = (uint64_t*) (h + 1);
            size_t first_found_idx = bitmap_size / 8 - as_nscasq((uint64_t) bitmap, UINT64_MAX, bitmap_size / 8) - 1;
            
            /* if found  */
            if(bitmap[first_found_idx] != UINT64_MAX)
            {
                size_t pg_idx = 64 * first_found_idx + FFS(~bitmap[first_found_idx]);
                while(pg_idx * MEM_PS < h->size                      /* still in segment */
                   && (!below || h->padr + pg_idx * MEM_PS < below)) /* respecting below */
                {
                    size_t fpg_idx = pg_idx;
                    printf("found free page from %lu\n", fpg_idx);
                    
                    while( pg_idx * MEM_PS < h->size                    /* still in segment */
                       && (!below || h->padr + pg_idx * MEM_PS < below) /* respecting below */
                       && (pg_idx - fpg_idx) * MEM_PS < size            /* didn't reach size */
                       && !BIT(bitmap[pg_idx / 64], pg_idx % 64))       /* page is clear */
                        ++pg_idx;
                    printf("stopped at page %lu\n", pg_idx);
                    /* pg_idx should be the first set bit after fpg_idx */
                    if(!cont || (pg_idx - fpg_idx) * MEM_PS >= size) /* if size is enough */
                    {
                        // TODO: firgure out the math for bit range operations
                        for(size_t i = fpg_idx; i < pg_idx; ++i)
                            bitmap[i / 64] |= ((uint64_t)1 << i % 64);
                        alloc.header = h;
                        alloc.padr = h->padr + fpg_idx * MEM_PS;
                        alloc.size = (pg_idx - fpg_idx) * MEM_PS;
                        printf("end mem_ppalloc()={%016p,%016p,%05lu}\n", alloc.header, alloc.padr, alloc.size);
                        return alloc;
                    }
                    printf("looking for another free page segment\n");
                    // look for next clear bit
                    // doing it bit by bit, cause I'm too lazy to figure out the math for an nscasq
                    // TODO: firgure out the math
                    while(pg_idx * MEM_PS < h->size                     /* still in segment */
                       && (!below || h->padr + pg_idx * MEM_PS < below) /* respecting below */
                       && BIT(bitmap[pg_idx / 64], pg_idx % 64))        /* page is set */
                        ++pg_idx;
                }
            }
        }
        pmm_header_off += sizeof(mem_pseg_header) + bitmap_size;
    }
    printf("end mem_ppalloc()={%016p,%016p,%05lu}\n", alloc.header, alloc.padr, alloc.size);
    return alloc;
}

void mem_ppfree(mem_pallocation alloc)
{
    // TODO: firgure out the math
    // Doing it bit by bit, cause again, too lazy to figure out the math for bit range operations
    printf("begin mem_ppfree(%016p,%016p,%05lu)\n", alloc.header, alloc.padr, alloc.size);
    uint64_t* bitmap = (uint64_t*) (alloc.header + 1);
    size_t pg_idx = (size_t) (alloc.padr - alloc.header->padr) / MEM_PS;
    for(size_t i = 0; i < alloc.size / MEM_PS; ++i)
        bitmap[(pg_idx + i) / 64] &= ~((uint64_t)1 << (pg_idx + i) % 64);
    printf("end mem_ppfree()\n");
}
