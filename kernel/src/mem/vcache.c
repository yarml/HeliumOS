#include <stdio.h>

#include"vcache.h"

// This pointer, after memory initialization, should point
// to 4 consecutive PDEs that are used for VCache
mem_pde_ref *vcache_pde;

// Pointer to the first PTE used by VCache. This PTE should be
// followed by another 2047 PTE
mem_pte *vcache_pte;

static uint16_t pde_free_pages(mem_pde_ref *pde)
{
  return mem_vpstruct_ptr_meta(pde) & 0xf;
}

static uint16_t pde_lazy_pages(mem_pde_ref *pde)
{
  return mem_vpstruct_ptr_meta(pde) >> 8 & 0xf;
}

static void pde_set_free(mem_pde_ref *pde, uint16_t free)
{
  uint16_t meta = mem_vpstruct_ptr_meta(pde);
  mem_vpstruct_ptr_set_meta(pde, meta & 0xff00 | free);
}

static void pde_set_lazy(mem_pde_ref *pde, uint16_t lazy)
{
  uint16_t meta = mem_vpstruct_ptr_meta(pde);
  mem_vpstruct_ptr_set_meta(pde, meta & 0x00ff | lazy << 8);
}

vcache_unit vcache_map(void *padr)
{
  printf("begin vcache_map(padr=%p)\n", padr);

  // Find a PDE which contains at least 1 free page

  mem_pde_ref *target_pde = 0;
  size_t pde_idx = 0;
  size_t free_pages_count = 0;

  for(size_t i = 0; i < PDE_COUNT; ++i)
  {
    if(free_pages_count = pde_free_pages(vcache_pde + i))
    {
      target_pde = vcache_pde + i;
      pde_idx = i;
      break;
    }
  }

  if(!target_pde)
  {
    // no free pages found, I am not in the state of mind to figure
    // out how things should be done
    // the idea is free lazy pages, then check again, but don't repeat code
    // TODO: future self, I am too depressed rn to figure out how to do this
    // please do this later
    // for now me will return {0, 0}

    vcache_unit u;

    u.ptr = padr;
    u.error = ERR_VCM_NPG_FOUND;

    printf("end vcache_map() -> ERR_VCM_NPG_FOUND\n");
  }

  // fuck life

  // Here we have a pde, we will decrement its `free` attribute
  pde_set_free(target_pde, free_pages_count - 1);

  // Now we iterate through all PTEs in this PDE and find one that is
  // not present
  mem_pte *target_pte = vcache_pte + pde_idx * 512;
  for(size_t i = 0; i < 512; ++i)
  {
    if(!target_pte[i].present)
    {
      target_pte = target_pte + i;
      break;
    }
  }

  // Mark the page as NOT lazy
  mem_vpstruct2_set_meta(target_pte, 0);

  // i forgot what i was doing :^]

}

void vcache_umap(vcache_unit unit)
{

}
