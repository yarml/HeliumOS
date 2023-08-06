#include <vcache.h>

#include <attributes.h>
#include <stdio.h>
#include <string.h>

#include <asm/invlpg.h>

#include "internal_vcache.h"

// This pointer, after memory initialization, should point
// to 4 consecutive PDEs that are used for VCache
mem_pde_ref *i_vcache_pde;

// Pointer to the first PTE used by VCache. This PTE should be
// followed by another 2047 PTE
mem_pte *i_vcache_pte;

vcache_unit vcache_map(void *padr) {
  prtrace_begin("vcache_map", "padr=%p", padr);

  // First thing, look if there is a lazy page pointing to this
  // exact physical address
  for (size_t pdei = 0; pdei < PDE_COUNT; ++pdei) {
    mem_pde_ref *pde        = i_vcache_pde + pdei;
    size_t       lazy_count = pde_lazy_pages(pde);

    if (!lazy_count) {
      continue;
    }

    for (size_t ptei = 0; ptei < 512; ++ptei) {
      mem_pte *pte = i_vcache_pte + pdei * 512 + ptei;
      if (pte->present && pte_age(pte) && pte->padr == (uintptr_t)padr >> 12) {
        pte_set_age(pte, 0);
        pde_set_lazy(pde, lazy_count - 1);

        vcache_unit u;
        u.error   = 0;
        u.pde_idx = pdei;
        u.pte_idx = ptei;
        u.ptr     = VCACHE_PTR + MEM_PS * (pdei * 512 + ptei);

        return u;
      }
    }
  }

  // Find a PDE which contains at least 1 free page

  mem_pde_ref *target_pde       = 0;
  size_t       pde_idx          = 0;
  size_t       free_pages_count = 0;

  // if 0, then no PDE has any lazy page
  // if not 0, then it is the number of lazy pages in the first PDE
  // that has any lazy page
  size_t lazy_pages_count = 0;

  mem_pte *target_pte = 0;
  size_t   pte_idx    = 0;

  for (size_t i = 0; i < PDE_COUNT; ++i) {
    if (!lazy_pages_count) {
      lazy_pages_count = pde_lazy_pages(i_vcache_pde + i);
    }

    free_pages_count = pde_free_pages(i_vcache_pde + i);
    if (free_pages_count) {
      target_pde = i_vcache_pde + i;
      pde_idx    = i;
      break;
    }
  }

  if (!target_pde) {
    if (!lazy_pages_count) {
      // No free pages were found, and no lazy pages could be freed either...
      // Return an error
      vcache_unit err;
      err.ptr     = 0;
      err.pte_idx = 0;
      err.pde_idx = 0;
      err.error   = ERR_VCM_NPG_FOUND;

      prtrace_end("vcache_map", "ERR_VCM_NPG_FOUND", 0);
      return err;
    }
    // Here, we should free lazy pages, then set target_pde again

    // iterate through all PDEs again, this time looking at their
    // lazy pages count, not free

    for (size_t i = 0; i < PDE_COUNT; ++i) {
      if ((free_pages_count = pde_lazy_pages(i_vcache_pde + i))) {
        size_t lazy_found_count = 0;
        // Iterate through all PTEs until we found all the lazy pages and
        // marked them as free
        target_pde = i_vcache_pde + i;
        for (size_t j = 0; j < 512; ++i) {
          // if we already found all the lazy pages, don't bother checking the
          // other ones, they are all used!
          if (lazy_found_count >= lazy_pages_count) {
            break;
          }

          mem_pte *pte = i_vcache_pte + i * 512;
          if (mem_vpstruct2_meta(pte))  // if it is lazy
          {
            // also set target_pte to this, so that we don't need to look for it
            // again after the flush is done
            target_pte = pte;
            pte_idx    = j;

            ++lazy_found_count;
            pte_set_age(pte, 0);
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
  if (!target_pte) {
    target_pte = i_vcache_pte + pde_idx * 512;
    // Now we iterate through all PTEs in this PDE and find one that is
    // not present
    for (size_t i = 0; i < 512; ++i) {
      if (!target_pte[i].present) {
        target_pte = target_pte + i;
        pte_idx    = i;
        break;
      }
    }
    // Mark the page as NOT lazy
    pte_set_age(target_pte, 0);
  }
  // Now that we have a PTE, we set it up
  memset(target_pte, 0, sizeof(*target_pte));

  target_pte->write   = 1;
  target_pte->global  = 1;
  target_pte->padr    = (uintptr_t)padr >> 12;
  target_pte->present = 1;

  void *ptr = VCACHE_PTR + MEM_PS * (pde_idx * 512 + pte_idx);

  as_invlpg((uint64_t)ptr);

  vcache_unit unit;

  unit.pde_idx = pde_idx;
  unit.pte_idx = pte_idx;
  unit.ptr     = ptr;
  unit.error   = 0;

  prtrace_end(
      "vcache_map", "SUCCESS", "ptr=%p,pde=%lu,pte=%lu", ptr, pde_idx, pte_idx
  );
  return unit;
}

void vcache_remap(vcache_unit unit, void *padr) {
  // Function stdout tracing disabled for vcache_remap because
  // it is called multiple times, especially during setup
  // printd(
  //   "begin vcache_remap(unit={ptr=%p,pde=%lu,pte=%lu}, padr=%p)\n",
  //   unit.ptr, unit.pde_idx, unit.pte_idx, padr
  // );

  mem_pte *pte = i_vcache_pte + unit.pde_idx * 512 + unit.pte_idx;

  // If this PTE already points to padr, skip the function
  if (pte->present && pte->padr == (uintptr_t)padr >> 12) {
    // printd("end vcache_remap() -> SKIP\n");
    return;
  }

  // Reconfigure the PTE from scratch
  memset(pte, 0, sizeof(*pte));

  pte->write   = 1;
  pte->global  = 1;
  pte->padr    = (uintptr_t)padr >> 12;
  pte->present = 1;

  void *ptr = VCACHE_PTR + MEM_PS * (unit.pde_idx * 512 + unit.pte_idx);

  as_invlpg((uint64_t)ptr);

  // printd("end vcache_remap() -> SUCCESS\n");
}

void vcache_umap(vcache_unit unit, void *id) {
  prtrace_begin(
      "vcache_umap",
      "unit={ptr=%p,pde=%lu,pte=%lu}, id=%p",
      unit.ptr,
      unit.pde_idx,
      unit.pte_idx,
      id
  );

  // First, check how many lazy pages the PDE has
  // If it is less than 127(the maximum), then the process is
  // straighforward. Mark this page as lazy with age 1, then return
  mem_pde_ref *pde = i_vcache_pde + unit.pde_idx;
  mem_pte     *pte = i_vcache_pte + unit.pde_idx * 512 + unit.pte_idx;

  // Pointer to the 512 PTEs pointed to by the PDE
  mem_pte *pde_pt = i_vcache_pte + 512 * unit.pde_idx;

  size_t lazy_count = pde_lazy_pages(pde);

  // This should be the majority of the first cases
  if (lazy_count < 63) {
    // Increment the age of the other lazy pages
    for (size_t i = 0; i < 512; ++i) {
      mem_pte *current_pte = pde_pt + i;
      size_t   age         = pte_age(current_pte);
      if (age && age < 1023) {
        pte_set_age(current_pte, age + i);
      }
    }
    if(id == VCACHE_AUTO_ID) {
      pte->padr = (uintptr_t) unit.ptr >> 12;
    } else if(id != VCACHE_NO_ID) {
      pte->padr = (uintptr_t)id >> 12;
    }
    pte_set_age(pte, 1);
    pde_set_lazy(pde, lazy_count + 1);
    prtrace_end("vcache_umap", "LAZY_PAGES_NOT_MAX", 0);
    return;
  }
  // If we already reached the maximum number of lazy pages
  // We need to free the oldest ones, also increasing the ages
  // of the ones that are still here
  size_t   age_sum    = 0;
  size_t   oldest_age = 0;
  mem_pte *oldest_pte = 0;

  // We do a first run removing the oldest lazy PTE
  for (size_t i = 0; i < 512; ++i) {
    size_t age = pte_age(pde_pt + i);
    if (age > oldest_age) {
      oldest_pte = pde_pt + i;
      oldest_age = age;
    }
    age_sum += age;
  }
  // We mark the page as free, not present
  pte_set_age(oldest_pte, 0);
  oldest_pte->present = 0;
  // Don't bother invlpg now

  // Stop considering the oldest page in the sum
  age_sum -= oldest_age;

  // Now compute the average age among lazy pages
  size_t av_age = age_sum / (lazy_count - 1);

  // number of lazy pages that will be marked free
  size_t removed_count = 0;

  // Now iterate again, freeing all lazy pages who are older than the average
  // and incrementing the ages of the ones that will stay
  for (size_t i = 0; i < 512; ++i) {
    mem_pte *current_pte = pde_pt + i;
    size_t   age         = pte_age(current_pte);

    // Make sure this is a lazy page not used/free
    if (age) {
      if (age >= av_age) {
        ++removed_count;
        // free page
        current_pte->present = 0;
        pte_set_age(current_pte, 0);
        // Don't bother invlpg
        continue;
        ;
      }

      // increment age if it's not maximum
      // in most situations, this condition is true
      if (age < 1023) {
        pte_set_age(current_pte, age + 1);
      }
    }
  }

  if (id != VCACHE_NO_ID) {
    pte->padr = (uintptr_t)id >> 12;
  }

  // Mark the target page as lazy
  pte_set_age(pte, 1);

  // Update the PDE with the number number of lazy pages
  // We don't count the oldest pte that was removed, because
  // it was replaced by the target pte
  pde_set_lazy(pde, lazy_count - removed_count);

  prtrace_end("vcache_umap", "SUCCESS", "free_count=%lu", removed_count);
}