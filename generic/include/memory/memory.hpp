#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <stdint.h>
#include <multiboot.h>
#include <kmath.hpp>

#define MEMORY_BITMAP   ((uint8_t*) 0x00000000)
#define MEMORY_MAX_SIZE (0xFFFFFFFF)
#define PAGES_MAX_COUNT ((MEMORY_MAX_SIZE / 4096 + 1) / 8)

namespace memory
{
    bool init(multiboot_info_t* mbt, uint32_t entries_count, interval* memory_map);
    void mark_page(  uint32_t page);
    void unmark_page(uint32_t page);
    uint32_t mark_next_page();
    uint32_t physical_page(uint32_t page);
}

#endif