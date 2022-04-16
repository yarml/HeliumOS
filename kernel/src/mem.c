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
    printf(" - %16p-%16p %7z %4e1:Free,2:ACPI,3:MMIO,#:Used\n", MMapEnt_Ptr(c), MMapEnt_Ptr(c) + MMapEnt_Size(c)-1, MMapEnt_Size(c) / 1024, MMapEnt_Type(c));
        if(MMapEnt_IsFree(c))
            mem_usable += MMapEnt_Size(c);
    }
    printf(" i %16p-%16p %7z\n", bootboot.initrd_ptr, bootboot.initrd_ptr + bootboot.initrd_size, bootboot.initrd_size);
    printf("Total usable memory: %z\n", mem_usable);


}
