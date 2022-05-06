#include <stdlib.h>
#include <errors.h>
#include <stdio.h>
#include <mem.h>
#include <debug.h>

// We only allocates units of multiples UNIT
#define UNIT (8)

typedef struct __block_header block_header;
typedef struct __unit_header unit_header;

#define BLOCK_MAGIC (0x08EA9A770CB70C20)
#define UNIT_MAGIC  (0x08EA9A770C541770)

// IMPORTANT WHEN MODIFYING THESE STRUCTURES
// unit_header.next should be of the same offset as block_header.funit
// sizeof(*) should be divisible by UNIT

struct __block_header
{
    uint64_t      magic ;
    uint64_t      pages ;
    unit_header*  funit ; // funit should be the same offset of unit_header.next
    uint64_t      lunit ; // largest unit in UNIT
    uint64_t      lcount; // count of units that have the largest size
    block_header* next  ;
    block_header* prev  ;
};

struct __unit_header
{
    uint64_t     magic;
    uint64_t     size ; // in UNIT
    unit_header* fnext; // free next
    unit_header* fprev; // free prev
    unit_header* rnext; // real next
    unit_header* rprev; // real prev
};

static block_header* first_block = 0;

// size in bytes
// sets prev and next to NULL
static block_header* alloc_block(size_t size)
{
    printf("Allocating new block\n");
    // size in pages considering the block header and first unit header
    size = ALIGN_UP(size + sizeof(block_header) + sizeof(unit_header), MEM_PAGE_SIZE) / MEM_PAGE_SIZE;
    block_header* b = (block_header*) 
        MAKE_CANONICAL(
            mem_vmm_alloc_range(
                MEM_ABS_IDX(VIRT_HEAP),
                0x8000000000,
                size
            ) * MEM_PAGE_SIZE
        )
    ;
    unit_header* u = (unit_header*) (((uint64_t)b) + sizeof(block_header));
    printf("Writing to b->magic\n");
    b->magic = BLOCK_MAGIC;
    printf("Can write to b->magic\n");
    b->pages = size;
    b->funit = u;
    b->lunit = (size * MEM_PAGE_SIZE - sizeof(block_header) - sizeof(unit_header)) / UNIT;
    b->lcount = 1;
    b->next = 0;
    b->prev = 0;

    u->magic = UNIT_MAGIC;
    u->size = b->lunit;
    u->fnext = 0;
    u->fprev = (unit_header*) b;
    u->rnext = 0;
    u->rprev = 0;
    printf("New block at: %p of lunit: %lu pages: %lu\n", b, b->lunit, b->pages);
    return b;
}

static void recalc_block(block_header* b)
{
    unit_header* u = b->funit;
    uint64_t largest = 0;
    uint64_t count = 0;
    int changed_twice = 0;
    while(u)
    {
        if(u->size > largest)
        {
            if(largest != 0)
                changed_twice = 1;
            largest = u->size;
        }
        if(u->size == largest)
            ++count;
        u = u->fnext;
    }
    b->lunit = largest;
    if(!changed_twice)
    {
        b->lcount = count;
        return;
    }
    u = b->funit;
    count = 0;
    while(u)
    {
        if(u->size == largest)
            ++count;
        u = u->fnext;
    }
    b->lcount = count;
}

void* malloc(size_t size)
{
    size = ALIGN_UP(size, UNIT);
    // first look for a block that has a large enough unit
    block_header* b = first_block;
    printf("malloc call\n");
    try_block:
    while(b)
    {
        if(b->lunit * UNIT >= size)
        {
            printf("Found a block large enough\n");
            unit_header* u = b->funit;
            // we look if any unit is exactly of the requested size(or not large enough to be splitted)
            unit_header* larger = 0;
            while(u)
            {
                if(size <= u->size * UNIT && u->size * UNIT <= size + sizeof(unit_header) + 1)
                {
                    u->fprev->fnext = u->fnext;
                    if(u->fnext)
                        u->fnext->fprev = u->fprev;
                    if(u->size == b->lunit)
                        if(!--(b->lunit))
                            recalc_block(b);
                    return (void*) ((uint64_t)u + sizeof(unit_header));
                }
                else if(u->size * UNIT >= size && (!larger || larger->size < u->size))
                    larger = u;
                u = u->fnext;
            }
            // otherwise, we look for a more large unit
            u = (unit_header*) (((uint64_t)b) + sizeof(block_header));
            while(u)
            {
                if(u->size * UNIT >= size && (!larger || larger->size < u->size))
                    larger = u;
                u = u->fnext;
            }
            // This should always be true, otherwise the block header needs recalculation
            if(larger)
            {
                printf("Found larger at %p\n", larger);
                u = larger;
                if(u->size == b->lunit)
                    if(!--(b->lunit))
                        recalc_block(b);
                // split
                printf("unit size %lx\n", u->size);
                printf("off %lx\n", sizeof(unit_header) + size);
                printf("remaining %lx\n", u->size - sizeof(unit_header) - size);
                u->size = size;
                unit_header* new = (unit_header*) (((uint64_t)u) + sizeof(unit_header) + size);
                printf("new at %p\n", new);
                new->magic = UNIT_MAGIC;
                printf("can write to new\n");
                new->size = size - sizeof(unit_header) / UNIT;
                new->fnext = u->fnext;
                new->fprev = u->fprev;
                new->rnext = u->rnext;
                new->rprev = u;

                if(new->fprev)
                    new->fprev->fnext = new;
                if(new->fnext)
                    new->fnext->fprev = new;
                if(new->rprev)
                    new->rprev->rnext = new;
                if(new->rnext)
                    new->rnext->rprev = new;

                u->rnext = new;
                u->fnext = new;
                printf("returning %lx\n", (uint64_t)u);
                return (void*) ((uint64_t)u + sizeof(unit_header));
            }
            else
                recalc_block(b);
        }
        b = b->next;
    }
    // no block large enough was found, allocate new one
    printf("Didnt find a large enough block, allocating a new one\n");
    b = alloc_block(size);
    printf("Allocated new block\n");
    first_block->prev = b;
    b->next = first_block;
    first_block = b;
    if(!b) // not enough space
        return 0;
    goto try_block;
}

void free(void* ptr)
{
    unit_header* u = ptr - sizeof(unit_header);
    if(u->magic != UNIT_MAGIC)
        error_virtual_memory_alloc("Tried to free an unallocated pointer");
    // Install ourselves in the free units linked list
    if(u->fprev)
        u->fprev->fnext = u;
    if(u->fnext)
        u->fnext->fprev = u;
    // merge if possible
    while(u->rnext && u->rnext == u->fnext)
    {
        u->rnext = u->rnext->rnext;
        u->fnext = u->rnext->fnext;
        u->size = u->rnext->size + sizeof(unit_header) / UNIT;
    }
    while(u->rprev && u->fprev == u->rprev)
    {
        u = u->rprev;
        u->rnext = u->rnext->rnext;
        u->fnext = u->rnext->fnext;
        u->size = u->rnext->size + sizeof(unit_header) / UNIT;
    }
}
