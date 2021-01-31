#include <dev/framebuffer.hpp>
#include <dev/serial.hpp>
#include <dev/pic.hpp>
#include <debug/debug.hpp>
#include <utils.hpp>
#include <interrupts/idt.hpp>

extern "C" void kmain(void) 
{
    dbg << "called kmain\n";
    setup_idt();
    framebuffer::clear();
    framebuffer::set_curs(0);
}