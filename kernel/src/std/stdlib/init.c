#include <stdlib.h>

#include "internal_stdlib.h"

block_header *i_stdlib_heap_header = 0;

void __init_stdlib()
{
  i_stdlib_heap_header = i_stdlib_alloc_block(INITIAL_HEAP_SIZE);

  // This is for testing. Apparently, this exposes a very serious bug in palloc
  i_stdlib_alloc_block(INITIAL_HEAP_SIZE);
}

