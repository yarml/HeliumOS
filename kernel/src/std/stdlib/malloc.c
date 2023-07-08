#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>

#include "internal_stdlib.h"

void *malloc(size_t size) {
  // malloc(). Malloc would work by searching for a block whose largest
  // free unit is at least the size of the requested bytes
  // Once we find a block, we try to find a unit that is the exact same
  // size as the request, otherwise, the unit whose size is the first larger one
  // If no blocks were found, we allocate a new one, add it to the block linked
  // list, then use that new block with a first unit that is exactly the
  // size we want, followed by a free unit taking the rest of the block
  // Block allocation should be minimized, because it is VERY expensive
  // mostly because I used shitty ways of finding virtual heap space, but
  // even if optimized, block allocation should be kept minimal

  // once a unit has been found, if it was the exact size, we remove it
  // from the linked list, but it stays in memory
  // If it was larger than the requested size+UNIT_SPLIT_DELTA, then a new
  // unit is inserted just after it, then it is removed, the unit is effectively
  // stripped down to the requested size

  // First thing first, return 0 if they asked for nothing
  if (!size) {
    return 0;
  }

  // Second thing second, round size to be a multiple of 16
  size = ALIGN_UP(size, 16);

  block_header *current_block = i_stdlib_heap_header;
  block_header *target_block  = 0;
  unit_header  *target_unit   = 0;

  while (current_block && !target_unit) {
    block_header *cb = current_block;
    current_block    = current_block->next;

    if (cb->largest_free_size < size) {
      continue;
    }

    unit_header *best_unit  = cb->largest_free;
    size_t       best_delta = cb->largest_free_size - size;
    // Go through all the free units in this block until
    // we find the block that has the closest size to the request

    unit_header *current_unit = cb->ffunit;

    while (best_delta && current_unit) {
      unit_header *cu = current_unit;
      current_unit    = current_unit->fnext;

      if (cu->size < size) {
        continue;
      }
      size_t this_delta = cu->size - size;
      if (this_delta < best_delta) {
        best_delta = this_delta;
        best_unit  = cu;
      }
    }

    // Now best_unit should contain the first best unit
    target_unit  = best_unit;
    target_block = cb;
  }

  // If we left the loop and we still don't have a target unit
  // then that means all blocks were used, and we need to allocate a new one

  if (!target_unit) {
    // There are three cases when allocating a new block; these aren't forced
    // technologically, I just want it the work this way
    // case I: size is less than 8M(INITIAL_HEAP_SIZE); we just allocate
    //         a block of 8M
    // case II: size is huge but less than 64M; we allocate a block that
    //          is double the requested size
    // case III: size is even more huge; Allocate a block just for this unit

    block_header *new_block;
    size_t        alloc_size = 0;
    if (size < INITIAL_HEAP_SIZE) {
      alloc_size = INITIAL_HEAP_SIZE;
    } else if (size < 64 * 1024 * 1024) {
      alloc_size = 2 * size;
    } else {
      alloc_size = size + sizeof(unit_header);
    }

    new_block = i_stdlib_alloc_block(alloc_size);

    if (!new_block) {
      errno = ENOMEM;
      return 0;
    }

    // We need to insert this new block into the linked list of blocks
    // We just insert it at the start
    i_stdlib_heap_header->prev = new_block;
    i_stdlib_heap_header       = new_block;

    // Now the target unit is the first unit of this newly allocated block
    target_unit  = i_stdlib_heap_header->ffunit;
    target_block = i_stdlib_heap_header;
  }

  // Original size of target unit, before any potential splitting
  size_t tu_org_size = target_unit->size;

  // Here target_unit is guarenteed to point to a unit that is at least
  // of the requested size

  // If unit can be split, split it, make target_unit exactly the requested size
  // while inserting after it a new unit
  if (size + UNIT_SPLIT_DELTA < target_unit->size) {
    unit_header *new_unit = (void *)(target_unit + 1) + size;
    memset(new_unit, 0, sizeof(*new_unit));

    new_unit->magic = UNIT_MAGIC;
    new_unit->size  = target_unit->size - size - sizeof(unit_header);
    new_unit->block = target_block;
    new_unit->flags = UNITF_FREE;

    // Insert new_unit into the free linked list & all linked list
    new_unit->next = target_unit->next;
    new_unit->prev = target_unit;

    new_unit->fnext = target_unit->fnext;
    new_unit->fprev = target_unit;

    target_unit->next  = new_unit;
    target_unit->fnext = new_unit;

    if (new_unit->next) {
      new_unit->next->prev = new_unit;
    }
    if (new_unit->fnext) {
      new_unit->fnext->fprev = new_unit;
    }

    target_unit->size = size;
  }

  // Check if unit is of largest size, then we need to update block data
  if (target_unit == target_block->largest_free) {
    unit_header *current_unit     = target_block->ffunit;
    unit_header *new_largest      = 0;
    size_t       new_largest_size = 0;

    while (current_unit) {
      unit_header *cu = current_unit;
      current_unit    = current_unit->fnext;

      if (cu == target_unit) {
        continue;
      }

      // If we find another free unit that has the same size, then
      // we don't need to check other units, that means there were more
      // than one unit of thelargest size
      if (cu->size == tu_org_size) {
        new_largest      = cu;
        new_largest_size = cu->size;
        break;
      }

      if (cu->size > new_largest_size) {
        new_largest      = cu;
        new_largest_size = cu->size;
      }
    }

    // Update block data about largest unit
    target_block->largest_free      = new_largest;
    target_block->largest_free_size = new_largest_size;
  }

  // Here, we are guarenteed that target_unit is EXACTLY the requested size
  // or if bigger, then not bigger enough to be split
  target_unit->flags &= ~UNITF_FREE;

  if (target_unit->fnext) {
    target_unit->fnext->fprev = target_unit->fprev;
  }

  // Setting the previous unit's next is special
  // because if there is no fprev, then we should set the block's ffunit
  if (target_unit->fprev) {
    target_unit->fprev->fnext = target_unit->fnext;
  } else {
    target_block->ffunit = target_unit->fnext;
  }

  return UNIT_PTR(target_unit);
}
