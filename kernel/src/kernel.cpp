#include <dev/serial.hpp>
#include <dev/framebuffer.hpp>
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
#include <dev/terminal.hpp>

extern "C" void kmain(multiboot_info_t* mbt)
{
    framebuffer::clear();
    framebuffer::set_curs(0);

    dbg << "Started kmain\n";
    if(!memory::init(mbt))
    {
        framebuffer::put_string(0, "Unsupported Memory Map: Nested memory areas are not supported yet");
        framebuffer::put_string(0, 1, "Or Not Enough Memory");
        return;
    }
    mbt = (multiboot_info_t*) ((uint32_t) mbt + 0xC0000000);
    dbg << "memory size: " << memory::memory_size * 4096 << '\n';
    pit::init(PIT_FREQ / 1000);
    terminal::init();
    setup_idt();
}

// 651264
// 2146369536

