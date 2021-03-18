#include <memory/memory.hpp>
#include <kmath.hpp>
#include <debug/debug.hpp>
#include <string.h>
#include <memory/gdt.hpp>

extern "C" void kernel_virtual_start();
extern "C" void kernel_virtual_end();
extern "C" void kernel_physical_start();
extern "C" void kernel_physical_end();
extern "C" void enable_paging(memory::page_directory_entry*);
extern "C" void reload_cr3();

namespace memory
{
    // In pages
    uint32_t k_vstart        = ((uint32_t) kernel_virtual_start ) / 4096    ; // Should be aligned
    uint32_t k_vend          = ((uint32_t) kernel_virtual_end   ) / 4096 + 1; // May not be aligned so We add 1 to be sure
    uint32_t k_pstart        = ((uint32_t) kernel_physical_start) / 4096    ;
    uint32_t k_pend          = ((uint32_t) kernel_physical_end  ) / 4096 + 1;
    uint8_t  physical_mem_manager_data[PAGES_MAX_COUNT / 8]                 ;
    uint32_t memory_size = 0                                                ;
    page_table_entry page_tables[1024 * 1024] __attribute__((aligned(4096)));
    page_directory_entry page_dir[1024]       __attribute__((aligned(4096)));

    bool init(multiboot_info_t* mbt)
    {
        dbg << "Kernel Size in memory: " << k_pend * 4096 - k_pstart *4096  << " bytes(" 
            << (k_pend * 4096 - k_pstart * 4096) / 1024 / 1024 << " MiB)\n";
        dbg << "Initializing memory\n";
        // FIXME: take the size member into account
        // I'll fix it when I face the problem of not fixing it
        uint32_t entries_count = mbt->mmap_length / sizeof(multiboot_memory_map_t);
        interval memory_map[entries_count];
        for(uint32_t i = 0; i < entries_count; i++)
        {
            
            multiboot_memory_map_t& e1 = mbt->mmap_addr[i];
            
            if(e1.type != MULTIBOOT_MEMORY_AVAILABLE || e1.addr.high != 0 || e1.len.high != 0)
            {
                memory_map[i] = interval::null();
                continue;
            }
            interval i1{ e1.addr.low, e1.addr.low + e1.len.low - 1 };
            interval interval_to_add = i1;
            for(uint32_t j = 0; j < entries_count; j++)
            {
                if(i != j)
                {
                    multiboot_memory_map_t& e2 = mbt->mmap_addr[j];
                    interval i2{ (uint32_t) (e2.addr.low), (uint32_t) (e2.addr.low) + e2.len.low - 1 };
                    
                    if(i1.contains(i2))
                    {
                        dbg << "Nested memory\n";
                        return false;
                    }
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
            // Making intervals aligned to 4096
            interval_to_add.start += 4096 * (interval_to_add.start % 4096 != 0) - (interval_to_add.start % 4096);
            interval_to_add.end   &= 0xFFFFF000;
            // We only need pages of start and end not all the address
            interval_to_add.start /= 4096;
            interval_to_add.end   /= 4096;
            interval_to_add.end   -= 1   ; // I am too lazy to think wether this is actually necessary

            if(!interval_to_add)
                interval_to_add = interval::null();
            memory_map[i] = interval_to_add;
            memory_size += interval_to_add.size();
        }
        dbg << "Setting up paging\n";
        // Setup paging
        // TODO: this should be in the architecture specific code
        dbg << "Clearing physical memory manager data\n";
        for(uint32_t i = 0; i < PAGES_MAX_COUNT / 8 / 4; i++)
            ((uint32_t*)physical_mem_manager_data)[i] = 0;
        dbg << "physical_mem_manager_data: " << physical_mem_manager_data << '\n';
        // Mark where the kernel is loaded physically as used
        dbg << "Marking kernel area as used\n";
        for(uint32_t i = k_pstart; i <= k_pend; i++)
            mark_page(i);
        dbg << "Initializing page directory\n";
        for(uint32_t i = 0; i < 1024; i++)
            page_dir[i] = page_directory_entry((page_table_entry*)((uint32_t) (&(page_tables[1024 * i])) - 0xC0000000), 0);
        // Map pages k_pstart:k_pend(+align 4M) -> k_vstart:k_vend(+align 4M)
        // And Identity map k_pstart:k_pend(align 4M)
        uint32_t k_pdstart = k_pstart / 1024;
        uint32_t k_pdend   = k_pend / 1024 + 1;
        uint32_t k_vdstart = k_vstart / 1024;
        uint32_t k_vdend = k_vend / 1024 + 1;
        dbg << "Mapping kernel(" << k_pdstart << ':' << k_pdend << ") to: " << k_vdstart << ':' << k_vdend << '\n';
        for(uint32_t i = 0; i < k_pdend - k_pdstart + 1; i++)
        {
            page_dir[k_vdstart + i].set_flags(page_struct_flags::PRESENT | page_struct_flags::RW);
            page_dir[k_pdstart + i].set_flags(page_struct_flags::PRESENT | page_struct_flags::RW);
            for(uint32_t j = 0; j < 1024; j++)
            {
                page_dir[k_vdstart + i][j] = page_table_entry(1024 * (k_pdstart + i) + j,
                                                              page_struct_flags::PRESENT | page_struct_flags::RW);
                page_dir[k_pdstart + i][j] = page_table_entry(1024 * (k_pdstart + i) + j,
                                                             page_struct_flags::PRESENT | page_struct_flags::RW);
            }
        }
        // Set gdt linear addresses to +0xC0000000
        enable_paging(page_dir);
        // Remove Identity mapping
        dbg << "Removing Identity mapping\n";
        for(uint32_t i = 0; i < k_pdend - k_pdstart + 1; i++)
        {
            for(uint32_t j = 0; j < 1024; j++)
            {
                page_dir[k_pdstart + i][j] = page_table_entry(0, 0);
            }
            page_dir[k_pdstart + i].set_flags(0);
        }
        reload_cr3();
        dbg << "Memory initialized\n";
        return true;
    }
    
    uint32_t mark_next_page()
    {
        for(uint32_t page = 0; page < PAGES_MAX_COUNT; page++)
        {
            uint8_t bit = page % 8;
            uint32_t byte = (page - bit) / 8;
            if(!((physical_mem_manager_data[byte] >> bit) & 1))
            {
                mark_page(page);
                return page;
            }
        }
        return INVALID_PAGE;
    }
    void mark_page(uint32_t page)
    {
        uint8_t bit = page % 8;
        uint32_t byte = (page - bit) / 8;
        physical_mem_manager_data[byte] |= 1 << bit;
    }
    void unmark_page(uint32_t page)
    {
        uint8_t bit = page % 8;
        uint32_t byte = (page - bit) / 8;
        physical_mem_manager_data[byte] &= ~(1 << bit);
    }
    page_table_entry::page_table_entry()
    {
        _flags = 0;
        _adr_low_free = 0;
        _adr_high = 0;
    }
    page_table_entry::page_table_entry(uint32_t phy_page, uint8_t flags)
    {
        _flags = flags;
        _adr_high     = (phy_page * 4096 & 0xFFFFF000) >> 16;
        _adr_low_free = (phy_page * 4096 & 0xFFFFF000) >> 8 ;
    }
    void page_table_entry::set_flags(uint8_t flags)
    {
        _flags = flags;
    }
    page_directory_entry::page_directory_entry()
    {
        _flags = 0;
        _adr_low_free = 0;
        _adr_high = 0;
    }
    page_directory_entry::page_directory_entry(page_table_entry* first, uint8_t flags)
    {
        _flags = flags;
        _adr_high     = ((uint32_t) first & 0xFFFFF000) >> 16;
        _adr_low_free = ((uint32_t) first & 0xFFFFF000) >> 8 ;
    }
    page_directory_entry::operator uint32_t()
    {
        return (_adr_high << 16) | (_adr_low_free << 8) | _flags;
    }
    void page_directory_entry::set_flags(uint8_t flags)
    {
        _flags = flags;
    }
    page_table_entry& page_directory_entry::at(uint32_t framen)
    {
        page_table_entry* pt = (page_table_entry*) (((uint32_t) _adr_high << 16 | (uint32_t)_adr_low_free << 8) + 0xC0000000);
        return *(pt + framen);
    }
    page_table_entry& page_directory_entry::operator[](uint32_t framen)
    {
        return at(framen);   
    }
}
