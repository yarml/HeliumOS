#include <dev/framebuffer.hpp>
#include <dev/serial.hpp>
#include <dev/pic.hpp>
#include <debug/debug.hpp>
#include <kutils.hpp>
#include <interrupts/idt.hpp>
#include <multiboot.h>
#include <string.h>
#include <algorithm>

extern "C" void kmain(multiboot_info_t* mbt)
{
    dbg << "Started kmain\n";
    setup_idt();
    framebuffer::clear();
    framebuffer::set_curs(0);
    // Detecting memory
    uint32_t available_mem = 0;
    uint32_t entries_count = mbt->mmap_length / sizeof(multiboot_memory_map_t);

    for(uint32_t i = 0; i < entries_count; i++)
    {
        multiboot_memory_map_t& entry = mbt->mmap_addr[i];
        dbg << i << ": " << entry.type << '\n';
        if(entry.type == MULTIBOOT_MEMORY_AVAILABLE)
                available_mem += entry.len.low;
    }
    dbg << "max(4, 5 ): " << (uint32_t) std::max(4, 5 ) << '\n';
    dbg << "min(8, 10): " << (uint32_t) std::min(8, 10) << '\n';
    dbg << "Total available memory: " << available_mem << " bytes(" << available_mem / 1024 /1024 << " Mib)\n";
    framebuffer::put_string(0, "HeliumOS");
}

// 651264
// 2146369536

