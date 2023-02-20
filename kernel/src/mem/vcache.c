#include <attributes.h>
#include <string.h>
#include <stdio.h>

#include <asm/invlpg.h>

#include"vcache.h"

// This pointer, after memory initialization, should point
// to 4 consecutive PDEs that are used for VCache
mem_pde_ref *vcache_pde;

// Pointer to the first PTE used by VCache. This PTE should be
// followed by another 2047 PTE
mem_pte *vcache_pte;

/* Implementation reference:
    - PDEs store two fields in their metadata:
      - `lazy` 7 bits: Count of lazy PTEs
      - `free` 9 bits: Count of free PTEs
    - PTEs store one field in their metadata:
      - `age` 11 bits: when 0, it means the page is free
                       otherwise, it is an integer number
                       that counts how many times that PTE
                       has been lazy, not implemented yet
 */

static uint16_t pde_free_pages(mem_pde_ref *pde)
{
  uint16_t meta = mem_vpstruct_ptr_meta(pde);
  printf("meta=%04x -> free= %d\n", meta, meta & 0x3ff);
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
      lazy_pages_count = pde_lazy_pages(vcache_pde + i);
    free_pages_count = pde_free_pages(vcache_pde + i);
    if(free_pages_count)
    {
      target_pde = vcache_pde + i;
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
      if((free_pages_count = pde_lazy_pages(vcache_pde + i)))
      {
        size_t lazy_found_count = 0;
        // Iterate through all PTEs until we found all the lazy pages and
        // marked them as free
        target_pde = vcache_pde + i;
        for(size_t j = 0; j < 512; ++i)
        {
          // if we already found all the lazy pages, don't bother checking the
          // other ones, they are all used!
          if(lazy_found_count >= lazy_pages_count)
          {
            break;
          }

          mem_pte *pte = vcache_pte + i * 512;
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
    target_pte = vcache_pte + pde_idx * 512;
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

void vcache_umap(vcache_unit unit)
{

}

// Initialization
void vcache_init()
{
  printf("begin vcache_init()\n");

  mem_pml4e *target_pml4e = i_pmlmax + ENTRY_IDX(3, VCACHE_PTR);
  mem_pdpte_ref *target_pdpt = 0;

  // TODO: if you see this function, we repeat a section of code 3 times
  // when im in the mood to do stuff correctly I will fix this

  // Chances are, this condition is always true
  if(!target_pml4e->present)
  {
    // Allocate a page to put the PDPT
    // The page should be under the border of 16G
    // because bootboot only guarentees that much memory to be
    // flat mapped
    mem_pallocation pdpt_alloc = mem_ppalloc(
      i_pmm_header,
      512 * sizeof(mem_pdpte_ref),
      0,
      true,
      (void *) ((uintptr_t) 16 * 1024*1024*1024)
    );

    if(pdpt_alloc.error)
      error_out_of_memory(
        "Could not allocate memory while initializing VCache!(PDPT)"
      );

    // Clear the newly allocated memory
    memset(pdpt_alloc.padr, 0, 512 * sizeof(mem_pdpte_ref));

    // Setup the PML4E
    memset(target_pml4e, 0, sizeof(*target_pml4e));

    target_pml4e->write = 1;
    target_pml4e->ss_padr = (uintptr_t) pdpt_alloc.padr >> 12;
    target_pml4e->present = 1;

    // Don't bother invlpg, it will be invalidated when a 4K page is mapped
  }

  target_pdpt = (mem_pdpte_ref *) SS_PADR(target_pml4e)
              + ENTRY_IDX(2, VCACHE_PTR);

  // Chances are, this condition is definitly always true
  if(!target_pdpt->present)
  {
    // Same deal as when PML4E was not present

    // Allocate place for the PD
    // Same as before, the page should be under 16G
    mem_pallocation pd_alloc = mem_ppalloc(
      i_pmm_header,
      512 * sizeof(mem_pde_ref),
      0,
      true,
      (void *) ((uintptr_t) 16 * 1024*1024*1024)
    );

    if(pd_alloc.error)
      error_out_of_memory(
        "Could not allocate memory while initializing VCache!(PD)"
      );

    // Clear the newly allocated memory
    memset(pd_alloc.padr, 0, 512 * sizeof(mem_pde_ref));

    // Setup the PDPTE
    target_pdpt->write = 1;
    target_pdpt->ss_padr = (uintptr_t) pd_alloc.padr >> 12;
    target_pdpt->present = 1;
    // and, same, no invlpg
  }

  // The PDEs, unlike the PML4E and PDPTE, MUST be not present
  vcache_pde = (mem_pde_ref *) SS_PADR(target_pdpt)
             + ENTRY_IDX(1, VCACHE_PTR);
  for(size_t i = 0; i < PDE_COUNT; ++i)
    if(vcache_pde[i].present)
      error_inv_state("VCache target PDEs were found in an invalid state");

  // Initialize the PDEs
  for(size_t i = 0; i < PDE_COUNT; ++i)
  {
    mem_pde_ref *pde = vcache_pde + i;

    // Same deal with PML4E and PDPTE
    mem_pallocation pt_alloc = mem_ppalloc(
      i_pmm_header,
      512 * sizeof(mem_pde_ref),
      0,
      true,
      (void *) ((uintptr_t) 16 * 1024*1024*1024)
    );

    if(pt_alloc.error)
      error_out_of_memory(
        "Could not allocate memory while initializing VCache!(PT)"
      );

    memset(pt_alloc.padr, 0, 512 * sizeof(mem_pde_ref));

    pde->write = 1;
    pde->ss_padr = (uintptr_t) pt_alloc.padr >> 12;
    pde->present = 1;

    // Then, we mark that this PDE have 512 free PTs
    pde_set_free(pde, 512);
  }

  // Initiazlixe vcache_pte
  vcache_pte = (mem_pte *) SS_PADR(vcache_pde)
             + ENTRY_IDX(0, VCACHE_PTR); // this is probaly +0

  // vcache_pde and vcache_pte are set to their physical addresses
  // vcache_map will work until the flat mapping is removed
  // BUT, we can use it to map these pages!

  // We have PDE_COUNT(4) pages to map for the PTs
  // TODO: I hope i can forgive myself for what i am about to do
  // but i know that, in the current state of the machine, vcache_map
  // will return consecutive pages after each call
  // so i will use that knowledge to map the PTs
  // FIXME: please asap, this is a sin, and i don't want to die
  mem_pte *vpte = 0;
  for(size_t i = 0; i < PDE_COUNT; ++i)
  {
    vcache_unit vu = vcache_map(vcache_pte + 512 * i);
    if(vu.error)
      error_general("VCache init", "Could not map vcache PT");

    // This has to do with the TODO/FIXME, we check that the address
    // we got back from vcache_map is what was expected
    // if i forget about the mess i made here, and change the implementation
    // of vcache_map, this would save me some hours probably
    if(vu.ptr != VCACHE_PTR + MEM_PS * i)
      error_inv_state(
        "Time to fix this ugly reliance on vcache_map behavior"
        "(" __FILE__ ":" AS_STRING(__LINE__) ")"
      );

    if(i == 0)
      vpte = vu.ptr;
  }
  vcache_pte = vpte;

  vcache_unit pde_unit = vcache_map(vcache_pde);
  if(pde_unit.error)
    error_general("VCache init", "Could not map vcache PD");
  vcache_pde = pde_unit.ptr;

  // Last but not least, map i_ppmlmax into virtual memory

  vcache_unit pmlmax_unit = vcache_map(i_ppmlmax);
  if(pmlmax_unit.error)
    error_general("VCache init", "Could not map PMLMAX");
  i_pmlmax = pmlmax_unit.ptr;

  printf("end vcache_init(pde=%p,pte=%p)\n", vcache_pde, vcache_pte);
}
