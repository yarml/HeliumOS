#include <stdint.h>
#include <stdio.h>
#include <utils.h>
#include <mem.h>

#include "internal_mem.h"

errno_t mem_vmap(void* vadr, void* padr, size_t size, int flags)
{
    printf("begin mem_vmap(%016p, %016p, %lu, %b)\n", vadr, padr, size, flags);
    if((uintptr_t) vadr % MEM_PS
    || (uintptr_t) padr % MEM_PS)
        return MAPE_ALN;

    int target_order = 0;

    if(flags & MAPF_P2M)
        target_order = 1;
    else if(flags & MAPF_P1G)
        target_order = 2;
    
    size_t mapped = 0;
    while(mapped < size)
    {        
        int order = 3;

        mem_vpstruct_ptr* i_target_entry = i_pml4 + ENTRY_IDX(order, vadr);

        while(order > target_order)
        {
            if(!i_target_entry->present) /* create new entry */
            {
                
            }
            i_target_entry = (mem_vpstruct_ptr*) SS_PADR(i_target_entry) + ENTRY_IDX(order, vadr);
            --order;
        }


    }
    return 0;
}

errno_t mem_vumap(void* vadr, size_t size)
{
    return 0;
}

