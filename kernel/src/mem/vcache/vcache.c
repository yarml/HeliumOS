#include <attributes.h>
#include <string.h>
#include <stdio.h>

#include <asm/invlpg.h>

#include"vcache.h"

// This pointer, after memory initialization, should point
// to 4 consecutive PDEs that are used for VCache
mem_pde_ref *i_vcache_pde;

// Pointer to the first PTE used by VCache. This PTE should be
// followed by another 2047 PTE
mem_pte *i_vcache_pte;

/* Implementation reference:
    - PDEs store two fields in their metadata:
      - `lazy` 7 bits: Count of lazy PTEs
      - `free` 9 bits: Count of free PTEs
    - PTEs store one field in their metadata:
      - `age` 11 bits: when 0, it means the page is free
                       otherwise, it is an integer number
                       that counts how many times that PTE
                       has been lazy
 */

static uint16_t pde_free_pages(mem_pde_ref *pde)
{
  uint16_t meta = mem_vpstruct_ptr_meta(pde);
  return meta & 0x3ff;
}

static uint16_t pde_lazy_pages(mem_pde_ref *pde)
{
  return mem_vpstruct_ptr_meta(pde) >> 10 & 0x7;
}

static void pde_set_free(mem_pde_ref *pde, uint16_t free)
{
  uint16_t meta = mem_vpstruct_ptr_meta(pde);
  mem_vpstruct_ptr_set_meta(pde, (meta & 0xfc00) | (free & 0x3ff));
}

static void pde_set_lazy(mem_pde_ref *pde, uint16_t lazy)
{
  uint16_t meta = mem_vpstruct_ptr_meta(pde);
  mem_vpstruct_ptr_set_meta(pde, (meta & 0x03ff) | (lazy & 0x7f) << 10);
}

vcache_unit vcache_map(void *padr)
{
  printf("begin vcache_map(padr=%p)\n", padr);

  // Find a PDE which contains at least 1 free page

  mem_pde_ref *target_pde = 0;
  size_t pde_idx = 0;
  size_t free_pages_count = 0;

  // if 0, then no PDE has any lazy page
  // if not 0, then it is the number of lazy pages in the first PDE
  // that has any lazy page
  size_t lazy_pages_count = 0;

  mem_pte *target_pte = 0;
  size_t pte_idx = 0;

  for(size_t i = 0; i < PDE_COUNT; ++i)
  {
    if(!lazy_pages_count)
      lazy_pages_count = pde_lazy_pages(i_vcache_pde + i);
    free_pages_count = pde_free_pages(i_vcache_pde + i);
    if(free_pages_count)
    {
      target_pde = i_vcache_pde + i;
      pde_idx = i;
      break;
    }
  }

  if(!target_pde)
  {
    if(!lazy_pages_count)
    {
      // No free pages were found, and no lazy pages could be freed either...
      // Return an error
      vcache_unit err;
      err.ptr = 0;
      err.pte_idx = 0;
      err.pde_idx = 0;
      err.error = ERR_VCM_NPG_FOUND;

      printf("end vcache_map() -> ERR_VCM_NPG_FOUND\n");
      return err;
    }
    // Here, we should free lazy pages, then set target_pde again

    // iterate through all PDEs again, this time looking at their
    // lazy pages count, not free

    for(size_t i = 0; i < PDE_COUNT; ++i)
    {
      if((free_pages_count = pde_lazy_pages(i_vcache_pde + i)))
      {
        size_t lazy_found_count = 0;
        // Iterate through all PTEs until we found all the lazy pages and
        // marked them as free
        target_pde = i_vcache_pde + i;
        for(size_t j = 0; j < 512; ++i)
        {
          // if we already found all the lazy pages, don't bother checking the
          // other ones, they are all used!
          if(lazy_found_count >= lazy_pages_count)
          {
            break;
          }

          mem_pte *pte = i_vcache_pte + i * 512;
          if(mem_vpstruct2_meta(pte)) // if it is lazy
          {
            // also set target_pte to this, so that we don't need to look for it
            // again after the flush is done
            target_pte = pte;
            pte_idx = j;

            ++lazy_found_count;
            mem_vpstruct2_set_meta(pte, 0);
            pte->present = 0;

            // Don't even bother invalidating the page, it will be invalidated
            // when mapped again
          }
        }
        // Mark all the previously lazy pages as free now
        pde_set_free(target_pde, lazy_pages_count);
        pde_set_lazy(target_pde, 0);

        // We only flush one PDE, do not bother with the other ones as that is
        // not needed now, and with chance, maybe they contain pages that
        // will soon be mapped again, so it's better they stay
        break;
      }
    }
  }

  // Here we have a pde, we will decrement its `free` attribute
  pde_set_free(target_pde, free_pages_count - 1);

  // If we went through a flush, then target_pte is already set
  if(!target_pte)
  {
    target_pte = i_vcache_pte + pde_idx * 512;
    // Now we iterate through all PTEs in this PDE and find one that is
    // not present
    for(size_t i = 0; i < 512; ++i)
    {
      if(!target_pte[i].present)
      {
        target_pte = target_pte + i;
        pte_idx = i;
        break;
      }
    }
    // Mark the page as NOT lazy
    mem_vpstruct2_set_meta(target_pte, 0);
  }
  // Now that we have a PTE, we set it up
  memset(target_pte, 0, sizeof(*target_pte));

  target_pte->write = 1;
  target_pte->global = 1;
  target_pte->padr = (uintptr_t) padr >> 12;
  target_pte->present = 1;

  void *ptr = VCACHE_PTR + MEM_PS * (pde_idx * 512 + pte_idx);

  as_invlpg((uint64_t) ptr);

  vcache_unit unit;

  unit.pde_idx = pde_idx;
  unit.pte_idx = pte_idx;
  unit.ptr = ptr;
  unit.error = 0;

  printf(
    "end vcache_map() -> SUCCESS(ptr=%p,pde=%lu,pte=%lu)\n",
    ptr, pde_idx, pte_idx
  );
  return unit;
}

void vcache_remap(vcache_unit unit, void *padr)
{
  mem_pte *pte = i_vcache_pte + unit.pde_idx * 512 + unit.pte_idx;

  // Reconfigure the PTE from scratch
  memset(pte, 0, sizeof(*pte));

  pte->write = 1;
  pte->global = 1;
  pte->padr = (uintptr_t) padr >> 12;
  pte->present = 1;

  void *ptr = VCACHE_PTR + MEM_PS * (unit.pde_idx * 512 + unit.pte_idx);

  as_invlpg((uint64_t) ptr);
}

void vcache_umap(vcache_unit unit)
{

}