#include <init/multiboot.h>
#include <mem/mem.hpp>
#include <debug.hpp>

extern "C" void kernel_main(multiboot_info_t* mbt)
{
    mem::init(mbt);
    debug::dbg_write("Hello, World!");
}
