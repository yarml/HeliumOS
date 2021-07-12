#include <debug.hpp>
#include <stddef.h>
#include <mem/gdt.hpp>

extern "C" void kernel_main()
{
    mem::gdt::init();
}
