#include <boot_info.h>
#include <string.h>
#include <smbios.h>
#include <stdio.h>
#include <debug.h>
#include <mem.h>
#include <fb.h>
#include <cpuid.h>

#include <collections/sorted_array.h>
#include <asm/io.h>
#include <math.h>
#include <stdlib.h>

static void print_info();

int compare(int* i1, int* i2)
{
       return *i1 - *i2;
}

void init()
{
       if(memcmp(&bootboot, "BOOT", 4))
       {
              // We don't know how we were loaded, as such, the only safe thing to do is LOOP
              as_outb(DEBUG_CONSOLE, 'B');
              LOOP;
       }
       
       {
              uint32_t a, b, c, d;
              __cpuid(1, a, b, c, d);
              if(b >> 24 != bootboot.bspid)
                     LOOP;
       }
       fb_init();
       print_info();
       mem_init();
       fb_dfb_init();
       mem_rm_identity_map();
       
       printf("Testing kernel heap\n");
       int* a = malloc(4);
       printf("%p ", a);
       *a = 10;
       printf("%d\n", *a);
       printf("Done testing kernel heap\n");

       LOOP;
}
static void print_info()
{
       printf("BOOTBOOT signature     : %10.4s        \n", bootboot.magic   );
       printf("BOOTBOOT struct size   : %10d          \n", bootboot.size    );
       printf("BOOTBOOT protocol      : %10d          \n", bootboot.protocol);
       printf("Number of cores        : %10d          \n", bootboot.numcores);
       printf("BSPID                  : %10d          \n", bootboot.bspid   );
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
