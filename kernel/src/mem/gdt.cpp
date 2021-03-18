#include <stdint.h>
#include <memory/gdt.hpp>
#include <debug/debug.hpp>

namespace gdt
{
    gdt_entry entries[GDT_SIZE];
    gdt gdtr;
}

extern "C" void load_gdt(gdt::gdt);

extern "C" void setup_gdt()
{
    gdt::entries[0] = gdt::gdt_entry(0, 0, 0, 0);
    gdt::entries[1] = gdt::gdt_entry(0x00000000, 0xFFFFFFFF, 
                                   gdt::gdt_access::PRESENT    | gdt::gdt_access::CODE_DATA | 
                                   gdt::gdt_access::EXEC       | gdt::gdt_access::RW        ,
                                   gdt::gdt_flags::GRANULARITY | gdt::gdt_flags::OP_SIZE    );
    gdt::entries[2] = gdt::gdt_entry(0x00000000, 0xFFFFFFFF,
                                   gdt::gdt_access::PRESENT                         | gdt::gdt_access::CODE_DATA|
                                   gdt::gdt_access::RW, gdt::gdt_flags::GRANULARITY | gdt::gdt_flags::OP_SIZE   );
    gdt::gdtr = { .size = sizeof(gdt::entries), .offset = gdt::entries };

    load_gdt(gdt::gdtr);
}