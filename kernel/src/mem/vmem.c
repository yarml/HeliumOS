#include <stdint.h>
#include <stdio.h>
#include <utils.h>
#include <mem.h>

#include <asm/invlpg.h>
#include <asm/ctlr.h>

#include "internal_mem.h"


errno_t mem_vmap(void *vadr, void *padr, size_t size, int flags)
{
    printf("begin mem_vmap(%016p, %016p, %lu, %b)\n", vadr, padr, size, flags);

    if(!size)
        return ERR_MEM_NULL_SIZE;

    // check if address is canonical
    // TODO: some architectures don't have non continuous virtual address
    // space like current forms of x64 with 48 bit addressing
    // When porting this should be disabled
    if((uintptr_t) (vadr           ) >> 47 != 0x1FFFF
    && (uintptr_t) (vadr           ) >> 47 != 0x00000
    && (uintptr_t) (vadr + size - 1) >> 47 != 0x1FFFF
    && (uintptr_t) (vadr + size - 1) >> 47 != 0x00000)
    {
        print("end mem_vmap() -> ERR_MEM_INV_VADR\n");
        return ERR_MEM_INV_VADR;
    }

    int target_order = 0;
    if(flags & MAPF_P1G)
        target_order = 2;
    else if(flags & MAPF_P2M)
        target_order = 1;

    if((uintptr_t) vadr % i_order_ps[target_order]
    || (uintptr_t) padr % i_order_ps[target_order])
    {
        print("end mem_vmap() -> ERR_MEM_ALN\n");
        return ERR_MEM_ALN;
    }

    size_t mapped = 0;

    // update the virtual tables using invlpg if:
    //  - the mapping is explicitly global
    //  - OR the mapping is in kernel space(implicitly global)
    //  - OR the total number of pages to map is less than the rlcr3
    //       threshold.
    int use_invlpg = flags & MAPF_G
        || vadr >= KVMSPACE
        || size / i_order_ps[target_order] < RLCR3_THRESHOLD;

    while(mapped < size)
    {
        int order = MAX_ORDER;

        mem_vpstruct_ptr *i_target_entry = i_pmlmax + ENTRY_IDX(order, vadr);

        while(order > target_order)
        {
            // check if we should allocate space for this structure's
            // substructures
            if(!i_target_entry->present)
            {
                // TODO: I do not like the fact that we throw away this
                // allocation struct and construct a new one when we are
                // deallocating It's just not how it's supposed to be done

                // What we do here is that we allocate
                // 512 entries(4096 bytes, 1 page) to contain all sub structures
                // of this entry
                mem_pallocation alloc = mem_ppalloc(
                    i_pmm_header,
                    512 * sizeof(mem_vpstruct_ptr),
                    0,
                    true,
                    0
                );
                if(alloc.error) // Out of space
                    error_out_of_memory(
                        "Couldn't allocate memory for VMM structres"
                    );
                *i_target_entry = i_default_vpstruct_ptr;
                i_target_entry->ss_padr = (uintptr_t) alloc.padr >> 12;
                i_target_entry->present = 1;

                // If we are mapping aren't in the VMM domain, then the new
                // pages we just allocated need to be mapped to their
                // corresponding places in the VMM super structure
                if(  (uintptr_t) vadr < (uintptr_t) KVMSPACE
                  || (uintptr_t) vadr >= (uintptr_t) (KVMSPACE + ))
            }
            if(flags & MAPF_SETUP) // We have identity paging at 0:16G, SS_PADR
                                   // will work without new mapping
                i_target_entry = (mem_vpstruct_ptr*) SS_PADR(i_target_entry)
                    + ENTRY_IDX(order - 1, vadr);
            else // If identity paging isn't guarenteed, we need to use the
                 // VMM CACHE to temporarely map the newly allocated entry to a
                 // vcache page
            {
                // TODO: when VCache is implemented, figure out what
                // i was trying to do here
            }
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
            rtentry->padr = (uintptr_t) padr >> 13; // reserved bits should be
                                                    // clear because addresses
                                                    // are aligned
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

            // With the PTE, we need to additionally check if we are
            // setting up for VCache, or this is a normal mapping
            if(flags & MAPF_VCSETUP)
                rtentry->present = 0;
            else
                rtentry->present = 1;
        }

        // skip if VCSETUP
        if(use_invlpg && !(flags & MAPF_VCSETUP))
            as_invlpg((uint64_t) vadr);

        vadr += i_order_ps[target_order];
        padr += i_order_ps[target_order];
        mapped += i_order_ps[target_order];
    }

    // Reload mappings using rlcr3 if we didnt do so using invlpg
    // skip if VCSETUP
    if(!use_invlpg && !(flags & MAPF_VCSETUP))
        as_rlcr3();

    print("end mem_vmap() -> SUCCESS\n");
    return 0;
}

errno_t mem_vumap(void *vadr, size_t size)
{
    return 0;
}
