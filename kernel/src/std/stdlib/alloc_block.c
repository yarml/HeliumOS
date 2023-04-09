#include <string.h>
#include <utils.h>
#include <stdio.h>
#include <mem.h>

#include "internal_stdlib.h"

#include "../src/mem/internal_mem.h"
#include "../src/mem/vcache/vcache.h"

// Find consecutive PTEs in the Kernel heap, then return a pointer to the
// memory region that has guarenteed page_count free pages after it
// returns 0 if not found, time to allocate new higher pages
// As such, kernel heap cannot be at address 0, because 0 would be a valid
// absolute index. But Im not planning on moving the heap to 0 anyway.
// This function assumes that the first PML4E is present
static void *find_consecutive_pages(size_t page_count, vcache_unit cache[3])
{
  // FIXME: The algorithm is shit, literally goes linearly through EVERY SINGLE
  // PTE in the heap until it finds a consecutive block
  // In the heap, there are 512^3 PTEs in total... do the math for worst case

  // Map the Kernel heap PDPT to cache
  mem_pml4e *target_pml4e = i_pmlmax + ENTRY_IDX(MAX_ORDER, KHEAP);

  if(!target_pml4e->present)
    return KHEAP;

  vcache_remap(cache[0], SS_PADR(target_pml4e));
  // Now cache[0].ptr points to an array of PDPTEs

  size_t counter = 0; // How many consecutive PTEs we found until now
  void *vptr = 0; // Pointer to the block of therequired minimum size
  for(size_t pdpte_idx = 0; pdpte_idx < 512; ++pdpte_idx)
  {
    mem_pdpte_ref *pdpte = (mem_pdpte_ref *) cache[0].ptr + pdpte_idx;

    // If not present, clear the counter & first_idx
    // then go to the next PDPTE
    if(!pdpte->present)
    {
      counter = 0;
      vptr = 0;
      continue;
    }

    // Map the PD pointed to by this PDPTE to cache
    vcache_remap(cache[1], SS_PADR(pdpte));

    // Iterate through the PDEs in this PD
    for(size_t pde_idx = 0; pde_idx < 512; ++pde_idx)
    {
      // Same logic for PDPTEs applies for PDES, only types change

      mem_pde_ref *pde = (mem_pde_ref *) cache[1].ptr + pde_idx;

      if(!pde->present)
      {
        counter = 0;
        vptr = 0;
        continue;
      }

      vcache_remap(cache[2], SS_PADR(pde));

      for(size_t pte_idx = 0; pte_idx < 512; ++pte_idx)
      {
        // Now we arrive at individual PTEs
        mem_pte *pte = (mem_pte *) cache[2].ptr + pte_idx;

        // If this PTE is a disappointment
        // counter to 0, yata yata yata
        if(!pte->present)
        {
          counter = 0;
          vptr = 0;
          continue;
        }
        // Else, this PTE is not a disappointment
        if(!vptr) // This is the first non disappointing PTE
        {
          // Compute the absolute index
          vptr =
            KHEAP +
            pte_idx * MEM_PS+
            pde_idx * 512 * MEM_PS +
            pdpte_idx * 512 * 512 * MEM_PS;
        }
        ++counter;

        // The grand final moment, or not
        if(counter == page_count)
          return vptr;
      }
    }
  }

  // If we leave all these loops without having returned, then that means
  // that we need to look in vstructures upper that PTEs, we do the same thing
  // but this time count free PDEs

  // We need to recalculate the number of pages we need, since we aren't using
  // PTEs anymore
  page_count = ALIGN_UP(page_count, 512) / 512;

  counter = 0;
  vptr = 0;

  // The code is duplicated
  // FIXME: Future me, here is a job for you

  for(size_t pdpte_idx = 0; pdpte_idx < 512; ++pdpte_idx)
  {
    mem_pdpte_ref *pdpte = (mem_pdpte_ref *) cache[0].ptr + pdpte_idx;

    // If not present, clear the counter & first_idx
    // then go to the next PDPTE
    if(!pdpte->present)
    {
      counter = 0;
      vptr = 0;
      continue;
    }

    // Map the PD pointed to by this PDPTE to cache
    vcache_remap(cache[1], SS_PADR(pdpte));

    // Iterate through the PDEs in this PD
    for(size_t pde_idx = 0; pde_idx < 512; ++pde_idx)
    {
      mem_pde_ref *pde = (mem_pde_ref *) cache[1].ptr + pde_idx;

      // Disappointment
      if(pde->present)
      {
        counter = 0;
        vptr = 0;
        continue;
      }

      if(!vptr)
      {
        vptr =
          KHEAP +
          pde_idx * 512 * MEM_PS +
          pdpte_idx * 512 * 512 * MEM_PS;
      }
      ++counter;

      if(counter == page_count)
        return vptr;
    }
  }

  // If we reach the end of that loop as well, then we need to look for free
  // PDPTEs. At least in this final loop, we don't remap the vcache
  page_count = ALIGN_UP(page_count, 512) / 512;
  counter = 0;
  vptr = 0;
  for(size_t pdpte_idx = 0; pdpte_idx < 512; ++pdpte_idx)
  {
    mem_pdpte_ref *pdpte = (mem_pdpte_ref *) cache[0].ptr + pdpte_idx;

    if(pdpte->present)
    {
      counter = 0;
      vptr = 0;
      continue;
    }

    if(!vptr)
    {
      vptr =
        KHEAP +
        pdpte_idx * 512 * 512 * MEM_PS;
    }

    ++counter;
    if(counter == page_count)
      return vptr;
  }

  // Last, if even that doesn't find us any space, then we ran out of heap space
  return 0; // Everything must come to an end
}

block_header *i_stdlib_alloc_block(size_t size, int flags)
{
  prtrace_begin("i_stdlib_alloc_block", "size=%lu", size);
  // Used to cache PDPTs, PDs, & PTs as we traverse the Vstructure of the heap
  vcache_unit ab_units[3];

  for(size_t i = 0; i < 3; ++i)
  {
    ab_units[i] = vcache_map(0);

    if(ab_units[i].error)
      error_out_of_memory("Could not allocate vcache memory");
  }

  // The requested size does not take into account that we need to allcoate
  // slightly more for the block header itself
  size += sizeof(block_header);
  // And then obviously we can only map in page sizes, so we up the size to
  // the first page size

  // Yes, the combination of these two actions does mean that sometimes,
  // we will allocate an entire page just because of that header, but do I look
  // like I care???
  size = ALIGN_UP(size, MEM_PS);


  // Travel the Vstructures at KHEAP, until we find consecutive pages
  // of the minimum required size
  void *vptr = find_consecutive_pages(size, ab_units);

  // If target_pde_idx is still zero, then that means we ran out of kernel
  // heap, this is probably impossible in practice
  if(!vptr)
    error_out_of_memory("Could not find free virtual pages in kernel heap");

  // Free the VCache units
  for(size_t i = 0; i < 3; ++i)
    vcache_umap(ab_units[i], 0);

  // Allocate physical pages
  size_t allocated = 0;
  while(allocated < size)
  {
    mem_pallocation alloc = mem_ppalloc(
      PALLOC_STD_HEADER,
      size - allocated,
      0,
      false,
      0
    );

    if(alloc.error)
      error_out_of_memory(
        "Could not allocate physical memory while "
        "trying to allocate kernel heap space!"
      );

    // Map the newly allocated physical pages to their place in the heap
    mem_vmap(vptr + allocated, alloc.padr, alloc.size, flags);
    allocated += alloc.size;
  }

  block_header *header = vptr;
  memset(header, 0, sizeof(*header));
  header->magic = BLOCK_MAGIC;
  header->block_size = size;

  // Setup the first unit
  unit_header *funit = (unit_header *) (header + 1);
  memset(funit, 0, sizeof(*funit));
  funit->magic = UNIT_MAGIC;
  funit->size = size - sizeof(block_header) - sizeof(unit_header);
  funit->block = header;
  funit->flags = UNITF_FREE;

  header->ffunit = funit;
  header->largest_free = funit;
  header->largest_free_size = funit->size;

  prtrace_end("i_stdlib_alloc_block", "SUCCESS", "header=%p", header);
  return header;
  // Done :)
}

// Public Heap block allocation function
void *alloc_block(size_t size, int flags)
{
  // i_stdlib_alloc_block adds sizeof(block_header) to the size, but we don't
  // need it here
  if(size > sizeof(block_header))
    size -= sizeof(block_header);

  void *block = i_stdlib_alloc_block(size, flags);

  // Remove headers just in case
  if(block)
    memset(block, 0, sizeof(block_header) + sizeof(unit_header));
  return block;
}
