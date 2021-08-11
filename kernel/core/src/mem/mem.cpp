#include <mem/mem.hpp>
#include <mem/gdt.hpp>
#include <utils/math.hpp>

#include "early.hpp"

namespace mem
{
    static early_heap::early_heap kearly_heap alignas(early_heap::UNIT_SIZE);
    void init(multiboot_info_t* mbt)
    {
        gdt::init();
    }
}
