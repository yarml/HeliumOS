#include <dev/serial.hpp>
#include <dev/framebuffer.hpp>
#include <dev/pic.hpp>
#include <debug/debug.hpp>
#include <kutils.hpp>
#include <interrupts/idt.hpp>
#include <multiboot.h>
#include <memory/memory.hpp>
#include <dev/keyboard.hpp>
#include <dev/pit.hpp>
#include <dev/io.hpp>
#include <kmath.hpp>
#include <dev/terminal.hpp>
#include <memory/heap.hpp>
#include <kutils/kstring.hpp>
#include <memory/page_list.hpp>

extern "C" void destroy_objects();

char* a = new char[10];

namespace memory::heap
{
    extern void list_journal();
}

extern "C" void kmain()
{
    dbg << "Started kmain\n";
    framebuffer::clear();
    framebuffer::set_curs(0);

    if(!memory::ready)
    {
        framebuffer::put_string(0, "Unsupported Memory Map: Nested memory areas are not supported yet");
        framebuffer::put_string(0, 1, "Or Not Enough Memory");
        return;
    }
    pit::init(PIT_FREQ / 100);
    terminal::init();
    setup_idt();
    new char[7];
    new char[2];

    delete[] a;

    memory::heap::list_journal();
    dbg << "Exiting kmain\n";
}

extern "C" void checck()
{
    dbg << "**********************Checking****************************\n";
    memory::heap::list_journal();
    dbg << "----------------------------------------------------------\n";
}
