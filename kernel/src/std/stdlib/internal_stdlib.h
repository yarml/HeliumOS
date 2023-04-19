#ifndef CSTD_INT_STDLIB_H
#define CSTD_INT_STDLIB_H

#include <stddef.h>
#include <mem.h>

struct BLOCK_HEADER;
typedef struct BLOCK_HEADER block_header;

struct UNIT_HEADER;
typedef struct UNIT_HEADER unit_header;

#define BLOCK_MAGIC (0x08EA9A770CB70C20)
#define UNIT_MAGIC  (0x08EA9A770C541770)

#define INITIAL_HEAP_SIZE (8 * 1024*1024)

struct BLOCK_HEADER
{
  size_t magic; // Verify that the pointer is indeed a block header

  size_t block_size; // Block size, header counted

  unit_header *ffunit; // First free unit
  unit_header *largest_free;
  size_t largest_free_size;

  block_header *next;
  block_header *prev;
};

struct UNIT_HEADER
{
  size_t magic; // Verify that the pointer is indeed a unit header

  size_t size; // Size, of bytes that this unit controls, not counting the bytes
               // the header itself takes

  size_t flags;

  block_header *block;

  unit_header *fnext;
  unit_header *fprev;

  unit_header *next;
  unit_header *prev;
};

#define UNITF_FREE (1<<1)

// Returns a pointer to the first unit header in of the block
#define BLOCK_FUNIT(b) ((unit_header *) ((b) + 1))

#define UNIT_PTR(u) ((void *) ((u) + 1))
#define PTR_UNIT(p) ((unit_header *) (p) - 1)

#define UNIT_SPLIT_DELTA (sizeof(unit_header) + 32)

// Allocates a number of pages from Kernel heap and assigns them to the block
// returned
// This new block should be integrated in the linked list by the caller function
// The block header is zeroed, and the magic number is placed
block_header *i_stdlib_alloc_block(size_t size);

extern block_header *i_stdlib_heap_header;

// Debug
void i_stdlib_malloc_print_state();

#endif