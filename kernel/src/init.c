#include <boot_info.h>
#include <string.h>
#include <smbios.h>
#include <stdio.h>
#include <debug.h>
#include <mem.h>
#include <fb.h>


#include <collections/sorted_array.h>
#include <asm/movs.h>

static void print_info();



int compare(int* i1, int* i2)
{
       return *i1 - *i2;
}

void init()
{
       // stupid way of """disabling""" all cores but one
       {
              uint8_t stack_top;
              if((uintmax_t) &stack_top < 0xFFFFFFFFFFFFFC00)
                     LOOP;
       }
       fb_init();
       // Testing sorted array
       int heap[24];
       for(int i = 0; i < 24; ++i)
              heap[i] = 0xABCDEF12;
       
       sorted_array sa;
       col_sa_init(&sa, heap, (fpt_diff) compare, sizeof(int), 24);
       printf("heap: %16p\n", heap);
       col_sa_vins(&sa, 7);
       for(size_t i = 0; i < sa.size + 2; ++i)
              printf("%x ", heap[i]);
       printf("\n");
       col_sa_vins(&sa, 6);
       for(size_t i = 0; i < sa.size + 2; ++i)
              printf("%x ", heap[i]);
       printf("\n");
       col_sa_vins(&sa, 5);
       for(size_t i = 0; i < sa.size + 2; ++i)
              printf("%x ", heap[i]);
       printf("\n");
       col_sa_vins(&sa, 4);
       for(size_t i = 0; i < sa.size + 2; ++i)
              printf("%x ", heap[i]);
       printf("\n");
       // LOOP;
       col_sa_vins(&sa, 2);
       for(size_t i = 0; i < sa.size + 2; ++i)
              printf("%x ", heap[i]);
       printf("\n");
       col_sa_vins(&sa, 1);
       for(size_t i = 0; i < sa.size + 2; ++i)
              printf("%x ", heap[i]);
       printf("\n");
       col_sa_vins(&sa, 3);
       for(size_t i = 0; i < sa.size + 2; ++i)
              printf("%x ", heap[i]);
       printf("\n");

       col_sa_idel(&sa, 1, 0);

       for(size_t i = 0; i < sa.size + 2; ++i)
              printf("%x ", heap[i]);
       printf("\n");
       // LOOP;
       print_info();
       mem_init();
       LOOP;
}
static void print_info()
{
       if(!memcmp(&bootboot, "BOOT", 4))
              printf("Valid bootboot structure.\n");
       else // We don't know how we were loaded, as such, the only safe thing to do is // LOOP
              // LOOP;
       printf("BOOTBOOT signature     : %10.4s        \n", bootboot.magic   );
       printf("BOOTBOOT struct size   : %10d          \n", bootboot.size    );
       printf("BOOTBOOT protocol      : %10d          \n", bootboot.protocol);
       printf("Number of cores        : %10d          \n", bootboot.numcores);
       printf("Local APIC Id          : %10d          \n", bootboot.bspid   );
       printf("Timezone               : %+9d          \n", bootboot.timezone);
       printf("UTC date               : %2x%2x/%2x/%2x\n"  ,
              bootboot.datetime[0], bootboot.datetime[1],
              bootboot.datetime[2], bootboot.datetime[3]);
       printf("UTC time               :   %2x:%2x:%2x \n"  ,
              bootboot.datetime[4], bootboot.datetime[5],
              bootboot.datetime[6]                      );
       printf("Initrd ptr|size        : %10p | %d \n", 
              bootboot.initrd_ptr, bootboot.size      );
       printf("fb type|ptr|size|WxH|sa: %10p | %d | %d | %dx%d | %x\n"  ,
              bootboot.fb_ptr, bootboot.size, bootboot.fb_type           ,
              bootboot.fb_width, bootboot.fb_height, bootboot.fb_scanline);
       printf("ACPI   ptr             : %10p          \n", bootboot.arch.x86_64.acpi_ptr);
       printf("SMBIOS ptr             : %10p          \n", bootboot.arch.x86_64.smbi_ptr);
       printf("EFI    ptr             : %10p          \n", bootboot.arch.x86_64.efi_ptr );
       printf("MP     ptr             : %10p          \n", bootboot.arch.x86_64.mp_ptr  );
}
