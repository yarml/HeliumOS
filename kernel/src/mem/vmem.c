#include <stdint.h>
#include <stdio.h>
#include <utils.h>
#include <mem.h>

#include <asm/invlpg.h>
#include <asm/ctlr.h>

#include "internal_mem.h"

static mem_vpstruct_ptr i_default_vpstruct_ptr = 
{
    .present  = 0, // present should be set after the address is set
    .write    = 1,
    .user     = 1,
    .pwt      = 0,
    .pcd      = 0,
    .accessed = 0,
    .free0    = 0,
    .ps       = 0,
    .free1    = 0,
    .ss_padr  = 0,
    .res0     = 0,
    .free2    = 0,
    .xd       = 0
};

// defult pde, pdpte with ps = 1
static mem_vpstruct i_default_vpstruct =
{
    .present  = 0, // present should be set after the address is set
    .write    = 0,
    .user     = 0,
    .pwt      = 0,
    .pcd      = 0,
    .accessed = 0,
    .dirty    = 0,
    .ps       = 1,
    .global   = 0,
    .free0    = 0,
    .pat      = 0,
    .padr     = 0,
    .res0     = 0,
    .free2    = 0,
    .prot_key = 0,
    .xd       = 0
};

// default pte
static mem_vpstruct2 i_default_vpstruct2 =
{
    .present  = 0,
    .write    = 0,
    .user     = 0,
    .pwt      = 0,
    .pcd      = 0,
    .accessed = 0,
    .dirty    = 0,
    .pat      = 0,
    .global   = 0,
    .free0    = 0,
    .padr     = 0,
    .res0     = 0,
    .free2    = 0,
    .prot_key = 0,
    .xd       = 0
};

errno_t mem_vmap(void* vadr, void* padr, size_t size, int flags)
{
    printf("begin mem_vmap(%016p, %016p, %lu, %b)\n", vadr, padr, size, flags);
    
    if(!size)
        return ERR_MEM_NULL_SIZE;

    // check if address is canonical
    // TODO: some architectures don't have non continuous virtual address
    // space like current forms x64 with 48 bit addressing
    // When porting this should be disabled
    if((uintptr_t) (vadr           ) >> 47 != 0x1FFFF
    && (uintptr_t) (vadr           ) >> 47 != 0x00000
    && (uintptr_t) (vadr + size - 1) >> 47 != 0x1FFFF
    && (uintptr_t) (vadr + size - 1) >> 47 != 0x00000)
        return ERR_MEM_INV_VADR;


    int target_order = 0;
    if(flags & MAPF_P1G)
        target_order = 2;
    else if(flags & MAPF_P2M)
        target_order = 1;
    
    if((uintptr_t) vadr % i_order_ps[target_order]
    || (uintptr_t) padr % i_order_ps[target_order])
        return ERR_MEM_ALN;
    
    size_t mapped = 0;

    while(mapped < size)
    {
        int order = MAX_ORDER;

        mem_vpstruct_ptr* i_target_entry = i_pmlmax + ENTRY_IDX(order, vadr);

        while(order > target_order)
        {
            // check if we should allocate space for this structure's substructures
            if(!i_target_entry->present)
            {
                // TODO: I do not like the fact that we throw away this allocation struct
                // and construct a new one when we are deallocating
                // It's just not how it's supposed to be done

                // What we do here is that we allocate 512 entries(4096 bytes, 1 page) to contain all sub structures of this entry
                mem_pallocation alloc = mem_ppalloc(i_pmm_header, 512 * sizeof(mem_vpstruct_ptr), 0, true, 0);
                if(alloc.error) // Out of space
                    error_out_of_memory("Couldn't allocate memory for VMM structres");
                *i_target_entry = i_default_vpstruct_ptr;
                i_target_entry->ss_padr = (uintptr_t) alloc.padr >> 12;
                i_target_entry->present = 1;
            }
            i_target_entry = (mem_vpstruct_ptr*) SS_PADR(i_target_entry) + ENTRY_IDX(order - 1, vadr);
            printf("%d\n", order - 1);
            --order;
        }
        // i_target_entry points to the target entry
        // and is of type mem_vpstructX
        // and order = target_order
        
        if(target_order) // a pdpt or pde
        {
            mem_vpstruct* rtentry = (mem_vpstruct*) i_target_entry;
            *rtentry = i_default_vpstruct;
            rtentry->write = (flags & MAPF_W) != 0;
            rtentry->user = (flags & MAPF_U) != 0;
            rtentry->global = vadr >= KVMSPACE || (flags & MAPF_G) != 0;
            rtentry->padr = (uintptr_t) padr >> 13; // reserved bits should be clear because addresses are aligned
            rtentry->present = 1;
        }
        else // a pte
        {
            mem_vpstruct2* rtentry = (mem_vpstruct2*) i_target_entry;
            *rtentry = i_default_vpstruct2;
            rtentry->write = (flags & MAPF_W) != 0;
            rtentry->user = (flags & MAPF_U) != 0;
            rtentry->global = vadr >= KVMSPACE || (flags & MAPF_G) != 0;
            rtentry->padr = (uintptr_t) padr >> 12;
            rtentry->present = 1;
        }
        // update the virtual tables
        if(flags & MAPF_G || vadr >= KVMSPACE || size / i_order_ps[target_order] < RLCR3_THRESHOLD)
            as_invlpg((uint64_t) vadr);
        
        vadr += i_order_ps[target_order];
        padr += i_order_ps[target_order];
        mapped += i_order_ps[target_order];
    }

    if(!(flags & MAPF_G || vadr >= KVMSPACE || size / i_order_ps[target_order] < RLCR3_THRESHOLD))
        as_rlcr3();

    return 0;
}

errno_t mem_vumap(void* vadr, size_t size)
{
    return 0;
}
