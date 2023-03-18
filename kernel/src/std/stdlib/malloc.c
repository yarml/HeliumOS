#include <stdlib.h>



void *malloc(size_t size)
{
  // malloc(). Malloc would work by searching for a block whose largest
  // unit is at least the size of the requested bytes
  // Once we find a block, we find try to find a unit that is the exact same
  // size as the request, oterwise, the unit whose size is the first largr one
  // If no blocks were found, we allocate a new one, add it to the block linked
  // list, then use that new block with a first unit that is exactly the
  // size we want, followed by a free unit taking the rest of the block
  // Block allocation should be minimized, because it is VERY expensive
  // mostly because I used shitty ways of finding virtual heap space, but
  // even if optimized, block allocation should be kept minimal

  // once a unit has been found, if it was the exact size, we remove it
  // from the linked list, but it stays in memory
  // If it was larger thanthe requested size+sizeof(unit_header), then a new
  // unit is inserted just after it, then it is removed, the unit is effectively
  // stripped down to the requested size



  return 0;
}
