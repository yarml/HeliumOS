#include <stdlib.h>
#include <stdio.h>

#include "internal_stdlib.h"

void i_stdlib_malloc_print_state()
{
  size_t block_count = 0;
  block_header *current_block = i_stdlib_heap_header;

  while(current_block)
  {

    printf(
      "Block #%lu (%lz) [ffunit=%p]:\n",
      block_count, current_block->block_size, current_block->ffunit
    );

    unit_header *current_unit = BLOCK_FUNIT(current_block);
    size_t unit_count = 0;
    while(current_unit)
    {
      printf(
        "\tUnit #%lu (%lz) [%p]",
        unit_count, current_unit->size, current_unit
      );

      if(current_unit->flags & UNITF_FREE)
      {
        printf(
          " F[fprev=%p,fnext=%p]",
          current_unit->fprev,
          current_unit->fnext
        );
      }
      if(current_unit->size == current_block->largest_free_size)
        printf(" X");
      if(current_unit == current_block->largest_free)
        printf(" U");

      printf("\n");
      ++unit_count;
      current_unit = current_unit->next;
    }
    ++block_count;
    current_block = current_block->next;
  }
}
