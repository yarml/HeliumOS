#include <boot_info.h>
#include <debug.h>
#include <cpuid.h>
#include <mem.h>

void __init_stdio();

int kmain();

// Initialize C stdlib then call kmain()
void _start()
{
    {
        uint32_t a, b, c, d;
        __cpuid(1, a, b, c, d);
        if(b >> 24 != bootboot.bspid)
                LOOP;
    }
    __init_stdio();
    mem_init();
    kmain();

    LOOP;
}
