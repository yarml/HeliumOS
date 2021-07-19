#include <mem/mem.hpp>
#include <mem/gdt.hpp>

namespace mem
{
    void init(multiboot_info_t* mbt)
    {
        gdt::init();
    }
}
