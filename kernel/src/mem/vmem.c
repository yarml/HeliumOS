#include <string.h>
#include <utils.h>
#include <stdio.h>
#include <mem.h>

#include <asm/invlpg.h>
#include <asm/ctlr.h>

#include "internal_mem.h"

#include "vcache/vcache.h"

errno_t mem_vmap(void *vadr, void *padr, size_t size, int flags)
{
  prtrace_begin(
    "mem_vmap",
    "vadr=%p,padr=%p,size=%lu,flags=%032b",
    vadr, padr, size, flags
  );

  if(!size)
    return ERR_MEM_NULL_SIZE;

  // check if address is canonical
  if(  (uintptr_t) (vadr           ) >> 47 != 0x1FFFF
    && (uintptr_t) (vadr           ) >> 47 != 0x00000
    && (uintptr_t) (vadr + size - 1) >> 47 != 0x1FFFF
    && (uintptr_t) (vadr + size - 1) >> 47 != 0x00000
  ) {
    prtrace_end("mem_vmap", "ERR_MEM_INV_VADR", 0);
    return ERR_MEM_INV_VADR;
  }

  // check that address isn't managed by another kernel system
  if(VCACHE_PTR <= vadr && vadr < (VCACHE_PTR + VCACHE_SIZE))
  {
    prtrace_end("mem_vmap", "ERR_MEM_MANAGED", 0);
    return ERR_MEM_MANAGED;
  }

  int target_order = 0;
  if(flags & MAPF_P1G)
    target_order = 2;
  else if(flags & MAPF_P2M)
    target_order = 1;

  if(  (uintptr_t) vadr % i_order_ps[target_order]
    || (uintptr_t) padr % i_order_ps[target_order]
  ) {
    prtrace_end("mem_vmap", "ERR_MEM_ALN", 0);
    return ERR_MEM_ALN;
  }

  // Align size with page size
  size = ALIGN_UP(size, i_order_ps[target_order]);

  // number of bytes mapped so far
  size_t mapped = 0;

  // update the virtual tables using invlpg if:
  //  - the mapping is explicitly global
  //  - OR the mapping is in kernel space(implicitly global)
  //  - OR the total number of pages to map is less than the rlcr3
  //       threshold.
  int use_invlpg = flags & MAPF_G
                || vadr >= KVMSPACE
                || size / i_order_ps[target_order] < RLCR3_THRESHOLD;

  // Allocate two pages in VCache that will be used to map substructures
  //The reason we need 2 pages is that, at any point in time, the algorithm
  // needs to access 2 pages at maximum
  // one page will contain the current target_entry, while the other will
  // contain the next one
  vcache_unit vmap_unit[2] = { vcache_map(0), vcache_map(0) };

  if(vmap_unit[0].error)
  {
    if(!vmap_unit[1].error)
      vcache_umap(vmap_unit[1], 0);
    prtrace_end("mem_vmap", "ERR_MEM_NO_VC_SPACE", 0);
    return ERR_MEM_NO_VC_SPACE;
  }

  if(vmap_unit[1].error)
  {
    if(!vmap_unit[0].error)
      vcache_umap(vmap_unit[0], 0);
    prtrace_end("mem_vmap", "ERR_MEM_NO_VC_SPACE", 0);
    return ERR_MEM_NO_VC_SPACE;
  }

  while(mapped < size)
  {
    int order = MAX_ORDER;
    mem_vpstruct_ptr *target_entry = i_pmlmax + ENTRY_IDX(order, vadr);

    while(order > target_order)
    {
      // If this VStruct is not present, we should allocate space
      // for it's substruct
      if(!target_entry->present)
      {
        mem_pallocation alloc = mem_ppalloc(
          i_pmm_header,
          512 * sizeof(mem_vpstruct_ptr),
          0,
          true,
          0
        );

        if(alloc.error)
        {
          prtrace_end("mem_vmap", "ERR_MEM_NO_PHY_SPACE", 0);
          return ERR_MEM_NO_PHY_SPACE;
        }

        vcache_remap(vmap_unit[order % 2], alloc.padr);

        // Clear the newly allocated page
        memset(vmap_unit[order % 2].ptr, 0, 512 * sizeof(mem_vpstruct_ptr));

        // Setup target_entry, target_entry should be
        // in the page vmap_unit[(order+1) % 2].ptr
        // or, the very first run, it should be in i_pmlmax
        memset(target_entry, 0, sizeof(*target_entry));
        target_entry->write = 1;

        // If the address is under KVMSPACE
        // we always set it as a user page ref, because it can contain
        // sub pages that are either the system's ot the user's
        // If I was sure the kernel had 0 vulnerability, I could set it as
        // as user page even if it is above KVMSPACE, and instead unset the user
        // flag in the individual page_map. But there could be vulnerabilities
        // in some other code, so a doubl protection can't hurt
        if(vadr < KVMSPACE)
          target_entry->user = 1;

        target_entry->ss_padr = (uintptr_t) alloc.padr >> 12;
        target_entry->present = 1;
      }

      // Increment substruct counter for this struct
      uint16_t current_subcount = mem_vpstruct_ptr_meta(target_entry);
      mem_vpstruct_ptr_set_meta(target_entry, current_subcount + 1);

      // So here, we are sure that the current VStruct entry is present
      // We map it's substructure into the coresponding VCache
      // If it is already mapped, vcache_remap will handle it and skip
      // re-remapping it
      vcache_remap(vmap_unit[order % 2], SS_PADR(target_entry));

      // Now we set the target entry to the next one in chain
      target_entry = (mem_vpstruct_ptr *) vmap_unit[order % 2].ptr
                   + ENTRY_IDX(order - 1, vadr);

      // Decrease order for the next loop
      --order;
    }

    // After the loop, target_entry is of type mem_p*e_map
    // either mem_pdte_map, mem_pde_map, or mem_pte_map
    // order == target_order

    // First thing is clearing them
    memset(target_entry, 0, sizeof(*target_entry));

    // We handle pdpte/pde in the same way
    if(target_order) // if pde/pdpte
    {
      // retype target_entry, l_entry for large_entry
      mem_vpstruct* l_entry = (mem_vpstruct *) target_entry;
      l_entry->ps = 1; // Mark this page as a map page not ref page
      l_entry->write = (flags & MAPF_W) != 0;
      if(vadr < KVMSPACE)
        l_entry->user = (flags & MAPF_U) != 0;
      l_entry->global = vadr >= KVMSPACE || (flags & MAPF_G) != 0;
      l_entry->padr = (uintptr_t) padr >> 13; // there is a reserved bit

      l_entry->present = 1;
    }
    else // target is PTE
    {
      mem_pte *pte = (mem_pte *) target_entry;
      pte->write = (flags & MAPF_W) != 0;
      if(vadr < KVMSPACE)
        pte->user = (flags & MAPF_U) != 0;
      pte->global = vadr >= KVMSPACE || (flags & MAPF_G) != 0;
      pte->padr = (uintptr_t) padr >> 12;

      pte->present = 1;
    }

    if(use_invlpg)
      as_invlpg((uint64_t) vadr);

    vadr += i_order_ps[target_order];
    padr += i_order_ps[target_order];
    mapped += i_order_ps[target_order];
  }

  // Unmap these pages with id 0 so that they could potentially be
  // reused in next calls to mem_vmap
  vcache_umap(vmap_unit[0], 0);
  vcache_umap(vmap_unit[1], 0);

  // Reload mappings using rlcr3 if we didnt do so using invlpg
  if(!use_invlpg)
    as_rlcr3();

  prtrace_end(
    "mem_vmap", "SUCCESS",
    "vadr=%p,padr=%p,size=%lu,flags=%032b", vadr, padr, size, flags
  );
  return 0;
}

errno_t mem_vumap(void *vadr, size_t size)
{
  // Trashy temporary implementation, that only marks the page as not present
  // This trashy temporary implementation does not bother freeing the physical
  // space used by virtual memory structures when it's currently unneeded
  // Me lazy rn, I just want it to work
  // FIXME: Hey, when you're not lazy anymore, properly implement this!!!

  // Actually, why even bother marking the page as unpresent at this point
  // Just return 0 who cares rn, i wanna work on malloc now

  return 0;
}
