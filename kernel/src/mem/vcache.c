#include <string.h>
#include <utils.h>

#include <asm/scas.h>

#include "internal_mem.h"
#include "vcache.h"

/* 2 bits per vcache page
 * values were intelligently selected to simplify the search algorithm
 *   - 00: Undefined
 *   - 01: Free
 *   - 10: Lazy unmapped
 *   - 11: Used
 */


size_t i_vcp_bitmap[VCP_BITMAP_LEN];
size_t i_vclp_table[VCACHE_LAZY_PAGES];
size_t i_vcache_lazy_count;

static void internal_map_vcp(void *vadr, void *padr);

vcache_unit vcache_map(void *padr)
{
    // index of first qword that has a free vcp
    size_t free_word_idx = -1;

    // At each iteration of this loop
    // free_word_idx is expected to be 1 less
    // than the first next free_word_idx that can be tested
    // For example, in the first iteration, any value
    // from [0;VCP_BITMAP_LEN) can be used, so
    // free_word_idx is -1
    while(1)
    {
        free_word_idx =
            (VCP_BITMAP_LEN - free_word_idx + 1)
          + as_nscasq(
                (uint64_t)(i_vcp_bitmap + free_word_idx + 1),
                UINT64_MAX,
                VCP_BITMAP_LEN - free_word_idx + 1
            )
          - 1;

        if(free_word_idx == -1)
        {
            if(!i_vcache_lazy_count)
            {
                // All pages are used and no page is lazily available!
                vcache_unit evcu;
                evcu.ptr = 0;
                evcu.error = ERR_VCM_NPG_FOUND;
                return evcu;
            }
            // Save the index of the first lazy page
            size_t flp_idx = i_vclp_table[0];
            vcache_flush();

            // Set free_word_idx to be the index of the first lazy page
            // that is now free after the flush

            free_word_idx = flp_idx / (8 * sizeof(size_t) / 2);
        }

        // Check that the found word actually contains a free page
        // not just a lazy page

        size_t word = i_vcp_bitmap[free_word_idx];

        // Index of the first 0 bit in the current word
        int fz_idx;

        while(1)
        {
            fz_idx = FFS(~word);

            // If this word only had lazy and used pages, move to the next one
            if(fz_idx == -1)
                break;

            // check if the index is odd or even
            // If the index of the 0 is even, then this is a free page
            // If it was odd, then this was just a lazy page
            // this is thanks to having chosen intellignemt values

            if(fz_idx % 2) // lazy page
            {
                // Artificially set the bit to one in the
                // temporary variable word then loop again
                // The next loop, we will see if there is
                // another 0 that can pottentially be
                // a free page

                word |= 1UL << fz_idx;
                continue;
            }

            // If execution arrives here, then we successfully found
            // a vcp that can be used
            size_t vcp_idx = (free_word_idx * 8 * sizeof(size_t) + fz_idx) / 2;
            void *ptr = VCACHE_PTR + MEM_PS * vcp_idx;
            internal_map_vcp(ptr, padr);

            vcache_unit vcu = { ptr, 0  };
            return vcu;
        }
    }
}

void vcache_umap(vcache_unit unit)
{
    if(i_vcache_lazy_count >= VCACHE_LAZY_PAGES)
        vcache_flush();

    // page index
    size_t vcp_idx = (unit.ptr - VCACHE_PTR) / MEM_PS;

    size_t bitmap_word_idx = vcp_idx / 32;
    size_t bitmap_word_off = vcp_idx % 32 * 2 + 1;

    // All we do now is flip the bit to 0 to set the page as lazy
    // instead of used. If this space is ever needed, the mapping algorithm
    // will actually free and invalidate the page table and all the slow stuff

    // It is important to note that, with this implementation, calling
    // the function on a free vcache unit will set one page to the undefined
    // state(00). Calling it on a lazy page will result in nothing.

    // However, both calls are invalid and result in undefined behaviour
    // in theory.

    i_vcp_bitmap[bitmap_word_idx] &= ~(1UL << bitmap_word_off);

    // Then, we need to add this to the vclp table
    i_vclp_table[i_vcache_lazy_count] = vcp_idx;
    ++i_vcache_lazy_count;

    // TADA!
}

void vcache_flush()
{
    // Flushing consists of:
    //  - Marking all the lazy pages as free
    //  - Flipping all their present flags to 0
    //  - Invalidate mappings
}



static void internal_map_vcp(void *vadr, void *padr)
{
    // This is a simpler version of mem_vmap
    // It is simpler because we can assume a number of things already

    // First, there are no flags, all pages are 4K pages with RW access and
    // not available to usermode

    // Second, all vmemory structures are already there and mapped to their
    // places in the vmem super structure and all that needs to be done is
    // setting the physical address, flipping the present flag then invalidate
    // the page
}
