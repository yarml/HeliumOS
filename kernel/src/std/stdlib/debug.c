#include <stdlib.h>
#include <stdio.h>

#include "internal_stdlib.h"

void i_stdlib_malloc_print_state()
{
  size_t block_count = 0;
  block_header *current_block = i_stdlib_heap_header;

  while(current_block)
  {

    printf("Block #%lu (%lz):\n", block_count, current_block->block_size);

    unit_header *current_unit = BLOCK_FUNIT(current_block);
    size_t unit_count = 0;
    while(current_unit)
    {
      printf("\tUnit #%lu (%lz)", unit_count, current_unit->size);

      if(current_unit->flags & UNITF_FREE)
        printf(" F");

      printf("\n");
      ++unit_count;
      current_unit = current_unit->next;
    }
    ++block_count;
    current_block = current_block->next;
  }
}
