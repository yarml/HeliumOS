#include <memory/memory.hpp>
#include <kmath.hpp>
#include <debug/debug.hpp>

namespace memory
{
    bool init(multiboot_info_t* mbt, uint32_t entries_count, interval* memory_map)
    {
        for(uint32_t i = 0; i < entries_count; i++)
        {
            multiboot_memory_map_t& e1 = mbt->mmap_addr[i];
            if(e1.type != MULTIBOOT_MEMORY_AVAILABLE)
            {
                memory_map[i] = interval::null();
                continue;
            }
            interval i1{ (uint32_t) (e1.addr.low), (uint32_t) (e1.addr.low) + e1.len.low - 1 };
            interval interval_to_add = i1;
            for(uint32_t j = 0; j < entries_count; j++)
            {
                if(i != j)
                {
                    multiboot_memory_map_t& e2 = mbt->mmap_addr[j];
                    interval i2{ (uint32_t) (e2.addr.low), (uint32_t) (e2.addr.low) + e2.len.low - 1 };
                    if(i1.contains(i2) || i2.contains(i1))
                        return false;
                    interval i12 = i1.intersection(i2);
                    if(i12 != interval::null())
                    {
                        if(i12.start == i2.start)
                            interval_to_add.end = i2.start - 1;
                        else if(i12.end == i2.end && e2.type != MULTIBOOT_MEMORY_AVAILABLE)
                            interval_to_add.start = i2.end + 1;
                    }
                }
            }
            interval_to_add.start += 4096 * (interval_to_add.start % 4096 != 0) - (interval_to_add.start % 4096);
            interval_to_add.end   &= 0xFFFF0000; // I feel superior
            interval_to_add.start /= 4096;
            interval_to_add.end   /= 4096;
            interval_to_add.end -= 1;

            if(!interval_to_add)
                interval_to_add = interval::null();
            memory_map[i] = interval_to_add;
        }
        // Clearing the first 128Kib for physical memory manager
        for(uint32_t page = 0; page < PAGES_MAX_COUNT; page++)
        {
            bool found = false;
            for(uint32_t i = 0; i < entries_count; i++)
            {
                if(memory_map[i].contains(page))
                {
                    unmark_page(page);
                    found = true;
                    break;
                }
            }
            if(!found)
                mark_page(page);           
        }
        // Mark the first 32 pages as used
        ((uint32_t*) MEMORY_BITMAP)[0] = 0xFFFFFFFF;
        

        return true;
    }
    uint32_t mark_next_page()
    {
        for(uint32_t page = 0; page < PAGES_MAX_COUNT; page++)
        {
            uint8_t bit = page % 8;
            uint32_t byte = (page - bit) / 8;
            if(!((MEMORY_BITMAP[byte] >> bit) & 1))
            {
                mark_page(page);
                return page;
            }
        }
        return 0;
    }
    void mark_page(uint32_t page)
    {
        uint8_t bit = page % 8;
        uint32_t byte = (page - bit) / 8;
        MEMORY_BITMAP[byte] |= 1 << bit;
    }
    void unmark_page(uint32_t page)
    {
        uint8_t bit = page % 8;
        uint32_t byte = (page - bit) / 8;
        MEMORY_BITMAP[byte] &= ~(1 << bit);
    }
}
