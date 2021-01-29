#include <devices/framebuffer.hpp>
#include <devices/serial.hpp>
#include <debug/debug.hpp>
#include <utils.hpp>

framebuffer fb  ;

extern "C" void kmain(void) 
{
    init_dbg();
    debug_msgln(MSG_INFO "called kmain");
    fb.clear();
    fb.set_curs(0);
    fb.put_string(0, "Hello, World!");
}