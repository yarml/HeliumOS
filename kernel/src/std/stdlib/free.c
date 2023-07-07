#include <stdio.h>
#include <stdlib.h>

#include "internal_stdlib.h"

void free(void *ptr) {
  // free(). Free works by first searching all the previous units for the
  // first free one, then sets the new fprev of the unit, then insert itself
  // in the linked list using fprev's fnext.
  // Then, check if the next unit is free, if so merge, then check if the
  // previous page is free, if so merge as well.
  // If after the merge, the new size of larger than the largest unit in the
  // block, update block data.

  if (!ptr) return;

  unit_header *target_unit = PTR_UNIT(ptr);

  if (target_unit->magic != UNIT_MAGIC) printd("free(): Pointer invalid\n");

  target_unit->fprev        = 0;

  // Go backwards until we find a free unit
  unit_header *current_unit = target_unit->prev;
  while (current_unit) {
    if (current_unit->flags & UNITF_FREE) {
      target_unit->fprev = current_unit;
      break;
    }
    current_unit = current_unit->prev;
  }

  // If we didn't find any free unit before this, then this is the new first
  // free unit now, update block
  if (!target_unit->fprev) {
    target_unit->fnext         = target_unit->block->ffunit;
    target_unit->block->ffunit = target_unit;
  } else  // Otherwise, we get fnext from fprev
  {
    target_unit->fnext        = target_unit->fprev->fnext;
    target_unit->fprev->fnext = target_unit;
  }

  // Set the previous of the next free unit to be this unit
  if (target_unit->fnext) target_unit->fnext->fprev = target_unit;

  // Merge with next unit if it is free
  if (target_unit->next && target_unit->fnext == target_unit->next) {
    target_unit->size += sizeof(unit_header) + target_unit->next->size;
    target_unit->next->magic = 0;  // Just in case
    target_unit->next        = target_unit->next->next;
    target_unit->fnext       = target_unit->fnext->fnext;
  }

  // Merge with previous unit if it is free
  if (target_unit->prev && target_unit->fprev == target_unit->prev) {
    unit_header *prev_unit = target_unit->prev;

    prev_unit->size += sizeof(unit_header) + target_unit->size;
    prev_unit->next    = target_unit->next;
    prev_unit->fnext   = target_unit->fnext;
    target_unit->magic = 0;

    target_unit        = prev_unit;
  }

  // Now check if the target unit, now free and merged with it's neighbours
  // is larger than the largest unit in the block, if so, update the block data
  if (target_unit->size > target_unit->block->largest_free_size) {
    target_unit->block->largest_free_size = target_unit->size;
    target_unit->block->largest_free      = target_unit;
  }

  // Finally, add the free flag to the target unit
  // (This is unnecessary in case the unit was merged with the prev,
  // but it doesn't hurt anyway)
  target_unit->flags |= UNITF_FREE;
}