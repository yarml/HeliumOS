#include <init/multiboot.h>
#include <mem/mem.hpp>

extern "C" void kernel_main(multiboot_info_t* mbt)
{
    mem::init(mbt);
}
