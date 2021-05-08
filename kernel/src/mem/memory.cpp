#include <memory/memory.hpp>
#include <kmath.hpp>
#include <debug/debug.hpp>
#include <memory/gdt.hpp>

extern "C" void kernel_virtual_start();
extern "C" void kernel_virtual_end();
extern "C" void kernel_physical_start();
extern "C" void kernel_physical_end();
extern "C" void enable_paging(memory::page_directory_entry *);
extern "C" void reload_cr3();

namespace memory
{
    bool ready = false;
    // In pages
    uint32_t k_vstart = ((uint32_t)kernel_virtual_start) / 4096; // Should be aligned
    uint32_t k_vend = ((uint32_t)kernel_virtual_end) / 4096 + 1; // May not be aligned so We add 1 to be sure
    uint32_t k_pstart = ((uint32_t)kernel_physical_start) / 4096;
    uint32_t k_pend = ((uint32_t)kernel_physical_end) / 4096 + 1;
    uint32_t kalloc_start = 0;
    uint8_t physical_mem_manager_data[PAGES_MAX_COUNT / 8];
    uint32_t memory_size = 0;
    page_table_entry page_tables[PAGE_TABLES_PER_DIR * PAGE_DIRS_COUNT] __attribute__((aligned(4096)));
    page_directory_entry page_dir[PAGE_DIRS_COUNT] __attribute__((aligned(4096)));

    bool init(multiboot_info_t *mbt)
    {
        dbg << "Kernel Size in memory: " << k_pend * 4096 - k_pstart * 4096 << " bytes("
            << (k_pend * 4096 - k_pstart * 4096) / 1024 / 1024 << " MiB)\n";
        dbg << "Initializing memory\n";
        // FIXME: take the size member into account
        // I'll fix it when I face the problem of not fixing it
        uint32_t entries_count = mbt->mmap_length / sizeof(multiboot_memory_map_t);
        kmath::interval memory_map[entries_count];
        for (uint32_t i = 0; i < entries_count; i++)
        {

            multiboot_memory_map_t &e1 = mbt->mmap_addr[i];

            if (e1.type != MULTIBOOT_MEMORY_AVAILABLE || e1.addr.high != 0 || e1.len.high != 0)
            {
                memory_map[i] = kmath::interval::null();
                continue;
            }
            kmath::interval i1{e1.addr.low, e1.addr.low + e1.len.low - 1};
            kmath::interval interval_to_add = i1;
            for (uint32_t j = 0; j < entries_count; j++)
            {
                if (i != j)
                {
                    multiboot_memory_map_t &e2 = mbt->mmap_addr[j];
                    kmath::interval i2{(uint32_t)(e2.addr.low), (uint32_t)(e2.addr.low) + e2.len.low - 1};

                    if (i1.contains(i2))
                    {
                        dbg << "Nested memory\n";
                        return false;
                    }
                    kmath::interval i12 = i1.intersection(i2);
                    if (i12 != kmath::interval::null())
                    {
                        if (i12.start == i2.start)
                            interval_to_add.end = i2.start - 1;
                        else if (i12.end == i2.end && e2.type != MULTIBOOT_MEMORY_AVAILABLE)
                            interval_to_add.start = i2.end + 1;
                    }
                }
            }
            // Making intervals aligned to 4096
            interval_to_add.start += 4096 * (interval_to_add.start % 4096 != 0) - (interval_to_add.start % 4096);
            interval_to_add.end &= 0xFFFFF000;
            // We only need pages of start and end not all the address
            interval_to_add.start /= 4096;
            interval_to_add.end /= 4096;
            interval_to_add.end -= 1; // I am too lazy to think wether this is actually necessary

            if (!interval_to_add)
                interval_to_add = kmath::interval::null();
            memory_map[i] = interval_to_add;
            memory_size += interval_to_add.size();
        }
        dbg << "Setting up paging\n";
        // Setup paging
        dbg << "Clearing physical memory manager data\n";
        for (uint32_t i = 0; i < PAGES_MAX_COUNT / 8 / 4; i++)
            ((uint32_t *)physical_mem_manager_data)[i] = 0;
        dbg << "physical_mem_manager_data: " << physical_mem_manager_data << '\n';
        // Mark where the kernel is loaded physically as used
        dbg << "Marking kernel area as used\n";
        for (uint32_t i = k_pstart; i <= k_pend; i++)
            mark_page(i);
        dbg << "Initializing page directory\n";
        for (uint32_t i = 0; i < 1024; i++)
            page_dir[i] = page_directory_entry((page_table_entry *)((uint32_t)(&(page_tables[1024 * i])) - 0xC0000000), page_struct_flags::PRESENT | page_struct_flags::RW);
        dbg << "Initializing page tables\n";
        for (uint32_t i = 0; i < PAGE_DIRS_COUNT * PAGE_TABLES_PER_DIR; i++)
            page_tables[i] = page_table_entry(0, 0);
        // Map pages k_pstart:k_pend(+align 4M) -> k_vstart:k_vend(+align 4M)
        // And Identity map k_pstart:k_pend(align 4M)
        uint32_t k_pdstart = k_pstart / 1024;
        uint32_t k_pdend = k_pend / 1024 + 1;
        uint32_t k_vdstart = k_vstart / 1024;
        uint32_t k_vdend = k_vend / 1024 + 1;
        kalloc_start = (k_vdend + 1) * 1024;
        dbg << "Mapping kernel(" << k_pdstart << ':' << k_pdend << ") to: " << k_vdstart << ':' << k_vdend << '\n';
        for (uint32_t i = 0; i < k_pdend - k_pdstart + 1; i++)
        {
            page_dir[k_vdstart + i].set_flags(page_struct_flags::PRESENT | page_struct_flags::RW);
            page_dir[k_pdstart + i].set_flags(page_struct_flags::PRESENT | page_struct_flags::RW);
            for (uint32_t j = 0; j < 1024; j++)
            {
                page_dir[k_vdstart + i][j] = page_table_entry(1024 * (k_pdstart + i) + j,
                                                              page_struct_flags::PRESENT | page_struct_flags::RW);
                page_dir[k_pdstart + i][j] = page_table_entry(1024 * (k_pdstart + i) + j,
                                                              page_struct_flags::PRESENT | page_struct_flags::RW);
            }
        }
        // Set gdt linear addresses to +0xC0000000
        dbg << "Enabling pagin\n";
        enable_paging(page_dir);
        dbg << "Paging enabled\n";
        // Remove Identity mapping
        dbg << "Removing Identity mapping\n";
        for (uint32_t i = 0; i < k_pdend - k_pdstart + 1; i++)
        {
            for (uint32_t j = 0; j < 1024; j++)
            {
                page_dir[k_pdstart + i][j] = page_table_entry(0, 0);
            }
            page_dir[k_pdstart + i].set_flags(page_struct_flags::PRESENT);
        }
        // Allocate first virtual page so that nullptr will really point to nothing
        page_tables[0] = page_table_entry(INVALID_PAGE, PRESENT);
        mbt = (multiboot_info_t *)((uint32_t)mbt + 0xC0000000);
        reload_cr3();
        dbg << "Memory initialized\n";
        return true;
    }
    uint32_t mark_next_page()
    {
        for (uint32_t page = 0; page < PAGES_MAX_COUNT; page++)
        {
            uint8_t bit = page % 8;
            uint32_t byte = (page - bit) / 8;
            if (!((physical_mem_manager_data[byte] >> bit) & 1))
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
    void map_page(uint32_t physical, uint32_t vrt)
    {
        page_tables[vrt] = page_table_entry(physical, page_struct_flags::PRESENT);
        reload_cr3();
    }
    void unmap_page(uint32_t vrt)
    {
        page_tables[vrt] = page_table_entry(0, 0);
        reload_cr3();
    }
    uint32_t kallocvp(uint32_t count)
    {
        if (count == 0)
            dbg << "Asked for 0 pages, may be a bug\n";
        uint32_t counter = 0;
        uint32_t start = INVALID_PAGE;
        for (uint32_t i = kalloc_start; i < PAGE_DIRS_COUNT * PAGE_TABLES_PER_DIR; i++)
        {
            page_table_entry f = page_tables[i];
            if (f.present())
            {
                counter = 0;
                start = INVALID_PAGE;
            }
            else
            {
                ++counter;
                if (start == INVALID_PAGE)
                    start = i;
                if (counter == count)
                {
                    for (uint32_t j = 0; j < count; j++)
                    {
                        uint32_t pp = mark_next_page();
                        if (pp == INVALID_PAGE)
                        {
                            // TODO: Should deallocate the already allocated physical pages
                            dbg << "Not enough physical address space\n";
                            return INVALID_PAGE;
                        }
                        map_page(pp, start + j);
                        page_tables[start + j].set_flag(page_struct_flags::RW);
                    }
                    return start;
                }
            }
        }
        return INVALID_PAGE;
    }
    void freevp(uint32_t start, uint32_t count)
    {
        for (uint32_t i = start; i < start + count; i++)
        {
            dbg << "Freeing " << start << '\n';
            dbg << "Unmapping " << page_tables[i].physical_page() << '\n';
            unmark_page(page_tables[i].physical_page());
            unmap_page(i);
        }
    }
    uint32_t size()
    {
        return memory_size * 4096;
    }
}

extern "C" void init_memory(multiboot_info_t *mbt)
{
    com1 = serial(SERIAL_COM1_BASE);
    dbg = debug(com1);
    memory::ready = false;
    for (uint32_t i = 0; i < PAGE_TABLES_PER_DIR * PAGE_DIRS_COUNT; i++)
        memory::page_tables[i] = memory::page_table_entry();
    for (uint32_t i = 0; i < PAGE_DIRS_COUNT; i++)
        memory::page_dir[i] = memory::page_directory_entry();
    memory::k_vstart = ((uint32_t)kernel_virtual_start) / 4096; // Should be aligned
    memory::k_vend = ((uint32_t)kernel_virtual_end) / 4096 + 1; // May not be aligned so We add 1 to be sure
    memory::k_pstart = ((uint32_t)kernel_physical_start) / 4096;
    memory::k_pend = ((uint32_t)kernel_physical_end) / 4096 + 1;
    memory::kalloc_start = 0;
    memory:: memory_size = 0;
    memory::ready = memory::init(mbt);
    dbg << "Returned\n";
}

