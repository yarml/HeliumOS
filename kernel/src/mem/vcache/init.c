#include <string.h>
#include <stdio.h>

#include "internal_vcache.h"
#include "vcache.h"

static void s_alloc_substruct(mem_vpstruct_ptr *vs)
{
  mem_pallocation alloc = mem_ppalloc(
    i_pmm_header,
    512 * sizeof(mem_vpstruct_ptr),
    0,
    true,
    (void *) ((uintptr_t) 16 * 1024*1024*1024)
  );

  if(alloc.error)
    error_out_of_memory(
      "Could not allocate memory while initializing VCache!"
    );

  // Clear newly allocated memory
  memset(alloc.padr, 0, 512 * sizeof(mem_vpstruct_ptr));

  // Setup the parent entry
  memset(vs, 0, sizeof(*vs));

  vs->write = 1;
  vs->ss_padr = (uintptr_t) alloc.padr >> 12;
  vs->present = 1;

  // Don't bother invlpg
}

void vcache_init()
{
  prtrace_begin("vcache_init", 0);

  mem_pml4e *target_pml4e = i_pmlmax + ENTRY_IDX(3, VCACHE_PTR);
  mem_pdpte_ref *target_pdpt = 0;

  // Chances are, this condition is always true
  if(!target_pml4e->present)
    s_alloc_substruct(target_pml4e);

  target_pdpt = (mem_pdpte_ref *) SS_PADR(target_pml4e)
              + ENTRY_IDX(2, VCACHE_PTR);

  // Chances are, this condition is definitly always true
  if(!target_pdpt->present)
    s_alloc_substruct(target_pdpt);

  i_vcache_pde = (mem_pde_ref *) SS_PADR(target_pdpt)
             + ENTRY_IDX(1, VCACHE_PTR);

  // The PDEs, unlike the PML4E and PDPTE, MUST be not present
  for(size_t i = 0; i < PDE_COUNT; ++i)
    if(i_vcache_pde[i].present)
      error_inv_state("VCache target PDEs were found in an invalid state");

  // Initialize the PDEs
  for(size_t i = 0; i < PDE_COUNT; ++i)
  {
    mem_pde_ref *pde = i_vcache_pde + i;
    s_alloc_substruct(pde);
    pde_set_free(pde, 512);
  }

  // Initiazlixe vcache_pte
  i_vcache_pte = (mem_pte *) SS_PADR(i_vcache_pde)
             + ENTRY_IDX(0, VCACHE_PTR); // this is probaly +0

  // vcache_pde and vcache_pte are set to their physical addresses
  // vcache_map will work until the flat mapping is removed
  // BUT, we can use it to map these pages!

  // We have PDE_COUNT(4) pages to map for the PTs
  // TODO: I hope i can forgive myself for what i am about to do
  // but i know that, in the current state of the machine, vcache_map
  // will return consecutive pages after each call
  // so i will use that knowledge to map the PTs
  // FIXME: please fix asap, this is a sin, and i don't want to die
  mem_pte *vpte = 0;
  for(size_t i = 0; i < PDE_COUNT; ++i)
  {
    vcache_unit vu = vcache_map(i_vcache_pte + 512 * i);
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
  i_vcache_pte = vpte;

  vcache_unit pde_unit = vcache_map(i_vcache_pde);
  if(pde_unit.error)
    error_general("VCache init", "Could not map vcache PD");
  i_vcache_pde = pde_unit.ptr;

  // Last but not least, map i_ppmlmax into virtual memory

  vcache_unit pmlmax_unit = vcache_map(i_ppmlmax);
  if(pmlmax_unit.error)
    error_general("VCache init", "Could not map PMLMAX");
  i_pmlmax = pmlmax_unit.ptr;

  prtrace_end(
    "vcache_init", "SUCCESS",
    "pde=%p,pte=%p", i_vcache_pde, i_vcache_pte
  );
}
