#include <dev/framebuffer.hpp>
#include <dev/serial.hpp>
#include <dev/pic.hpp>
#include <debug/debug.hpp>
#include <kutils.hpp>
#include <interrupts/idt.hpp>
#include <multiboot.h>
#include <string.h>
#include <memory/memory.hpp>
#include <dev/keyboard.hpp>
#include <dev/pit.hpp>
#include <dev/io.hpp>
#include <kmath.hpp>

extern "C" void kmain(multiboot_info_t* mbt)
{
    framebuffer::clear();
    framebuffer::set_curs(0);
    framebuffer::put_string(0, "HeliumOS");

    dbg << "Started kmain\n";

    // FIXME: take the size member into account
    uint32_t memory_entries_count = mbt->mmap_length / sizeof(multiboot_memory_map_t);
    interval memory_map[memory_entries_count];
    if(!memory::init(mbt, memory_entries_count, memory_map))
    {
        framebuffer::put_string(0, "Unsupported memory map: Nested memory areas are not supported yet");
        return;
    }
    dbg << "Available Memory: \n";
    uint32_t available_memory_size = 0;
    for(uint32_t i = 0; i < memory_entries_count; i++)
    {
        if(memory_map[i] != interval::null())
        {
            dbg << "\tBase Page: " << memory_map[i].start
                << ", End  Page: " << memory_map[i].end
                << ", Size: " << memory_map[i].size()
                << '\n';
            available_memory_size += memory_map[i].size() * 4096;
        }
    }
    dbg << "Total: " << available_memory_size << " bytes (" << available_memory_size / 1024 / 1024 << " Mib)\n";

    pit::init(PIT_FREQ / 1000);
    setup_idt();
}

// 651264
// 2146369536

