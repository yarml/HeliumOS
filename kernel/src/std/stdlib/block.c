#include <string.h>
#include <utils.h>
#include <stdio.h>
#include <mem.h>

#include "internal_stdlib.h"

#include "../src/mem/internal_mem.h"
#include "../src/mem/vcache/vcache.h"

block_header *i_stdlib_alloc_block(size_t size)
{
  // The requested size does not take into account that we need to allcoate
  // slightly more for the block header itself
  size += sizeof(block_header);

  mem_vseg seg = mem_alloc_vblock(size, MAPF_R | MAPF_W, KHEAP, KHEAP_SIZE);

  if(seg.error)
    return 0;

  void *vptr = seg.ptr;

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

  return header;
  // Done :)
}
