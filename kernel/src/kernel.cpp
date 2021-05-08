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

extern "C" void destroy_objects();

char* a = new char[10];



extern "C" void kmain()
{
    framebuffer::clear();
    framebuffer::set_curs(0);

    dbg << "Started kmain\n";
    if(!memory::ready)
    {
        framebuffer::put_string(0, "Unsupported Memory Map: Nested memory areas are not supported yet");
        framebuffer::put_string(0, 1, "Or Not Enough Memory");
        return;
    }
    pit::init(PIT_FREQ / 1000);
    terminal::init();
    setup_idt();

    //char* b = new char[5];

    delete[] a;
    //delete[] b;

    dbg << "Exiting kmain\n";
}
