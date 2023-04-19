#include <string.h>
#include <utils.h>
#include <stdio.h>
#include <mem.h>

#include "internal_stdlib.h"

#include "../src/mem/internal_mem.h"
#include "../src/mem/vcache/vcache.h"

block_header *i_stdlib_alloc_block(size_t size, int flags)
{
  prtrace_begin("i_stdlib_alloc_block", "size=%lu", size);

  // The requested size does not take into account that we need to allcoate
  // slightly more for the block header itself
  size += sizeof(block_header);
  // And then obviously we can only map in page sizes, so we up the size to
  // the first page size

  // Yes, the combination of these two actions does mean that sometimes,
  // we will allocate an entire page just because of that header, but do I look
  // like I care???
  size = ALIGN_UP(size, MEM_PS);

  void *vptr = mem_find_vsegment(size, KHEAP, (size_t) 512 * 1024*1024*1024);

  // If target_pde_idx is still zero, then that means we ran out of kernel
  // heap, this is probably impossible in practice
  if(!vptr)
    error_out_of_memory("Could not find free virtual pages in kernel heap");

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

void free_block(void *vadr, size_t size)
{
  // TODO: Implement free_block
}
