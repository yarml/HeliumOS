#include <stdlib.h>
#include <string.h>
#include <utils.h>

#include "internal_stdlib.h"

void *realloc(void *ptr, size_t size)
{
  // realloc(). Realloc has three cases
  // case I: the size difference is small enough to be ignored
  //         in which case, the function simply returns the same pointer in
  //          obviously only when new size is smaller than old size.
  // case II: the new size is smaller than the size of the unit, and the unit
  //          can be split. In which case, the unit is split, and the new unit
  //          is placed right after the original unit and marked as free and
  //          merged with the next unit if it also happens to be free
  // case III: The new size is larger than the requested size, there are three
  //           sub cases
  // case IIIa: The next unit is free and just large enough to hold the
  //            additional size without splitting the rest of it into
  //            another unit
  // case IIIb: The next unit is free and way larger than needed, in which case
  //            it is split to what we need, and the first half is merged with
  //            the unit of ptr
  // case IIIc: The next unit is not free, or non existant, or small,
  //            in which case, we allocate a new unit using malloc,
  //            copy the data and free ptr

  // Align size to 16 bytes
  size = ALIGN_UP(size, 16);

  // Case -1: size is 0, this is a free not realloc
  if(!size)
  {
    free(ptr);
    return 0;
  }
  // Case 0: ptr is NULL, this is an alloc, not realloc
  if(!ptr)
    return malloc(size);

  unit_header *target_unit = PTR_UNIT(ptr);

  if(size < target_unit->size)
  {
    // Check for case I
    size_t delta = target_unit->size - size;
    if(delta < UNIT_SPLIT_DELTA)
      return ptr; // Case I

    // Case II
    unit_header *new_unit = (void *) (target_unit + 1) + size;
    memset(new_unit, 0, sizeof(*new_unit));

    new_unit->magic = UNIT_MAGIC;
    new_unit->size = target_unit->size - size - sizeof(unit_header);
    new_unit->block = target_unit->block;
    new_unit->flags = UNITF_FREE;

    // Insert new_unit into the free linked list & all linked list
    new_unit->next = target_unit->next;

    new_unit->prev = target_unit;
    target_unit->next = new_unit;

    if(new_unit->next)
      new_unit->next->prev = new_unit;

    target_unit->size = size;

    // Search for previous free unit
    unit_header *current_unit = target_unit->prev;
    while(current_unit)
    {
      if(current_unit->flags & UNITF_FREE)
      {
        new_unit->fprev = current_unit;
        break;
      }

      current_unit = current_unit->prev;
    }

    // If we didn't find any free unit before, then that means this new split
    // unit is the new first free unit
    if(!new_unit->fprev)
    {
      new_unit->fnext = new_unit->block->ffunit;
      target_unit->block->ffunit = new_unit;
    }
    else
    {
      // If we did find a unit before this one, then we set
      // fnext of new_unit and fprev to reflect the new state of the list
      new_unit->fnext = new_unit->fprev->fnext;
      new_unit->fprev->fnext = new_unit;
    }
    if(new_unit->fnext)
      new_unit->fnext->fprev = new_unit;

    // Merge the new split unit with the next one if possible
    if(new_unit->next && new_unit->next == new_unit->fnext)
    {
      new_unit->size += sizeof(unit_header) + new_unit->next->size;
      new_unit->next = new_unit->next->next;
      new_unit->fnext = new_unit->fnext->fnext;
      new_unit->next->magic = 0; // Just in case
    }

    // Finally, check if this new split unit is bigger than what used to be the
    // biggest unit
    if(new_unit->size > new_unit->block->largest_free_size)
    {
      new_unit->block->largest_free = new_unit;
      new_unit->block->largest_free_size = new_unit->size;
    }
    return ptr;
  }

  // If execution reaches here, then that means we are in case III

  // delta > UNIT_SPLIT_DELTA guarenteed
  size_t delta = size - target_unit->size;

  // Check for case IIIc first, as it is the simplest
  if(
    !target_unit->next ||
    !(target_unit->next->flags & UNITF_FREE) ||
    target_unit->next->size + sizeof(unit_header) < delta
  ) {
    // next unit either does not exist
    // is not free
    // or too small
    void *new_ptr = malloc(size);
    if(!new_ptr)
      return 0;
    memcpy(new_ptr, ptr, target_unit->size);
    free(ptr);
    return new_ptr;
  }

  // Here, next unit is guarenteed to be free and at least
  // of large enough size

  // Now check for case IIIa
  if(target_unit->next->size < delta + UNIT_SPLIT_DELTA)
  {
    // We can merge the two units
    unit_header *next = target_unit->next;
    // Remove the unit that will be merged from the free linked list
    if(next->fprev)
      next->fprev->fnext = next->fnext;
    if(next->fnext)
      next->fnext->fprev = next->fprev;

    // If the unit that will be merged was the largest, then we need to find
    // the new largest one
    if(next->size == target_unit->block->largest_free_size)
    {
      size_t new_largest_size = 0;
      unit_header *new_largest = 0;
      unit_header *current_unit = target_unit->block->ffunit;
      while(current_unit)
      {
        unit_header *cu = current_unit;
        current_unit = current_unit->fnext;

        // Skip the unit we are merging
        if(cu == next)
          continue;

        // We found another unit that had the same size as the largest one
        if(cu->size == target_unit->block->largest_free_size)
        {
          target_unit->block->largest_free = cu;
          break;
        }

        if(cu->size > new_largest_size)
        {
          new_largest_size = cu->size;
          new_largest = cu;
        }
      }

      target_unit->block->largest_free = new_largest;
      target_unit->block->largest_free_size = new_largest_size;
    }

    // Remove next from the all units linked list
    target_unit->next = next->next;
    if(next->next)
      next->next->prev = target_unit;

    next->magic = 0; // No longer has the magic of a unit header
    target_unit->size += sizeof(unit_header) + next->size;

    return ptr;
  }

  // Here, it is case IIIb, we grow the target unit into the one just after it

  // This makes it so that when we eat from the next unit, the unit headers
  // are never overlapping

  unit_header *next = target_unit->next;

  // Save a copy of the old header in stack, because if delta is small
  // enough, the new next header can be overlapping the old header
  unit_header snext = *next;

  unit_header *new_next = (void *) next + delta;
  memset(new_next, 0, sizeof(*new_next));

  new_next->magic = UNIT_MAGIC;
  new_next->block = snext.block;
  new_next->size = snext.size - delta - sizeof(unit_header);
  new_next->flags = UNITF_FREE;

  // Remove the magic if it is not in the overlapping area
  if(delta > offsetof(unit_header, magic) + sizeof(next->magic))
    next->magic = 0;

  target_unit->size += delta;

  new_next->next = snext.next;
  new_next->prev = target_unit;
  target_unit->next = new_next;

  new_next->fnext = snext.next;
  new_next->fprev = snext.fprev;

  if(new_next->next)
    new_next->next->prev = new_next;

  if(new_next->fnext)
    new_next->fnext->fprev = new_next;

  if(new_next->fprev)
    new_next->fprev->fnext = new_next;
  else
    new_next->block->ffunit = new_next;


  // Update block data if the unit eaten was the largest one
  if(target_unit->block->largest_free == next)
  {
    unit_header *current_unit = new_next->block->ffunit;

    size_t new_largest_free_size = 0;
    unit_header *new_largest = 0;

    while(current_unit)
    {
      unit_header *cu = current_unit;
      current_unit = current_unit->fnext;

      if(cu->size == target_unit->block->largest_free_size)
      {
        new_largest = cu;
        new_largest_free_size = cu->size;
        break;
      }

      if(cu->size > new_largest_free_size)
      {
        new_largest_free_size = cu->size;
        new_largest = cu;
      }
    }

    target_unit->block->largest_free = new_largest;
    target_unit->block->largest_free_size = new_largest_free_size;
  }

  // If I didn't do any catastrophic mistakes, this should be it

  return ptr;
}
