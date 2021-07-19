#include <mem/mem.hpp>
#include <mem/gdt.hpp>
#include <debug.hpp>

namespace mem
{
    static early_heap::early_heap kearly_heap;
    void init(multiboot_info_t* mbt)
    {
        gdt::init();
    }
}
