#include <mem.h>

#include <boot_info.h>
#include <stddef.h>
#include <stdio.h>

void mem_init()
{
    size_t mmap_size = (bootboot.size - sizeof(BOOTBOOT)) / sizeof(MMapEnt) + 1;
    size_t mem_usable = 0;
    printf("Memory map entries(%d):\n", mmap_size);
    for(MMapEnt* c = &bootboot.mmap; mmap_size != 0; ++c, --mmap_size)
    {
        printf(" - %8p-%8p  %10d  %2d\n", MMapEnt_Ptr(c), MMapEnt_Ptr(c) + MMapEnt_Size(c)-1, MMapEnt_Size(c), MMapEnt_Type(c));
        if(MMapEnt_IsFree(c))
            mem_usable += MMapEnt_Size(c);
    }
    printf("Total usable memory: %dM\n", mem_usable / 1024 / 1024);

    
}
