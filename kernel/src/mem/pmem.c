#include <mem.h>
#include <mutex.h>
#include <stdint.h>
#include <stdio.h>
#include <utils.h>

#include <asm/scas.h>

#include "internal_mem.h"

/*
  Notes related to the implementation:
  * 0 means a free page
  * 1 means a used page
  * bitmap is aligned to 64 bits, last bits that don't map to any page should
    always marked as used
  * mem_ppfree doesn't protect from unsetting those last bits
  * mem_ppalloc however would never consider them for allocation even if they
    are marked as free
  * Just keep in mind, that if the implementation of mem_ppalloc changes, some
    bugs in the kernel may start appearing(which should be good i guess)
*/

static mutex pmm_lock;

mem_pallocation mem_ppalloc(
    void *pheader, size_t size, size_t alignment, bool cont, void *below
) {
  prtrace_begin(
      "mem_ppalloc",
      "pheader=%p, size=%lu, alignment=%lu, cont=%d, below=%p",
      pheader,
      size,
      alignment,
      cont,
      below
  );
  if (pheader == PALLOC_STD_HEADER) {
    pheader = i_pmm_header;
  }

  // We check the validity of the arguments first before acquiring the lock
  mem_pallocation alloc;
  alloc.header_off = 0;
  alloc.padr       = 0;
  alloc.size       = 0;
  alloc.error      = 0;

  if (!size) {
    alloc.error = ERR_MEM_NULL_SIZE;
    return alloc;
  }

  if (!alignment) {
    alignment = MEM_PS;
  } else if (alignment % MEM_PS) {
    alloc.error = ERR_MEM_ALN;
    return alloc;
  }

  mutex_lock(&pmm_lock);

  size_t alignment_p = alignment / MEM_PS;

  size_t pmm_header_off = 0;
  for (size_t i = 0; i < i_mmap_usable_len; ++i) {
    mem_pseg_header *h = pheader + pmm_header_off;

    if (h->magic != MEM_PSEG_MAGIC) {
      error_inv_state("Corrupted physical memory header");
    }

    size_t bitmap_size = BITMAP_SIZE(h->size);

    if ((!cont || h->size >= size)        /* big enough, without
                                             considering allocated pages */
        && (!below || h->padr < below)) { /* lowest address low enough */
      uint64_t *bitmap = (uint64_t *)(h + 1);
      size_t    first_found_idx =
          bitmap_size / 8 -
          as_nscasq((uint64_t)bitmap, UINT64_MAX, bitmap_size / 8) - 1;

      /* if found  */
      if (bitmap[first_found_idx] != UINT64_MAX) {
        int    ffs    = FFS(~bitmap[first_found_idx]);  // can't be -1
        size_t pg_idx = 64 * first_found_idx + ffs;
        pg_idx        = GALIGN_UP(pg_idx, alignment_p);  // we need to be
                                                         // aligned
        while (pg_idx * MEM_PS < h->size                 /* still in segment */
               && (!below || h->padr + pg_idx * MEM_PS < below
                  )                                      /* respecting below */
               && !BIT(bitmap[pg_idx / 64], pg_idx % 64) /* page is clear */
        ) {
          size_t fpg_idx = pg_idx;

          while (pg_idx * MEM_PS < h->size /* still in segment */
                 && (!below || h->padr + pg_idx * MEM_PS < below
                    )                                    /* respecting below */
                 && ((pg_idx - fpg_idx) * MEM_PS < size) /* didn't reach size */
                 && !BIT(bitmap[pg_idx / 64], pg_idx % 64) /* page is clear */
          ) {
            pg_idx += alignment_p;
          }

          size_t lpg_idx  = pg_idx - 1;
          size_t pg_count = lpg_idx - fpg_idx + 1;
          if (!cont || pg_count * MEM_PS >= size) { /* if size is enough */
            // A more efficient way to set bits than
            // the older implementation
            if (lpg_idx - fpg_idx + 1 < 64) {  // only one/two u64 to change
              if (ALIGN_DN(lpg_idx, 64) != ALIGN_DN(fpg_idx, 64)) {  // one u64
                bitmap[fpg_idx / 64] |=
                    BITRANGE(fpg_idx % 64, lpg_idx % 64 + 1);
              } else {  // two u64 to change
                bitmap[fpg_idx / 64] |= BITRANGE(fpg_idx % 64, 64);
                bitmap[lpg_idx / 64] |= BITRANGE(0, lpg_idx % 64 + 1);
              }
            } else {  // multiple u64s to set
              bitmap[fpg_idx / 64] |= BITRANGE(fpg_idx % 64, 64);
              bitmap[lpg_idx / 64] |= BITRANGE(0, lpg_idx % 64 + 1);
              // Set lpg_idx to not count the last few pages that have
              // only a hald word in the bitmap
              lpg_idx        = ALIGN_DN(lpg_idx, 64);
              size_t cpg_idx = ALIGN_UP(fpg_idx, 64);

              while (cpg_idx < lpg_idx) {
                bitmap[cpg_idx / 64] = UINT64_MAX;
                cpg_idx += 64;
              }
            }
            alloc.header_off = pmm_header_off;
            alloc.padr       = h->padr + fpg_idx * MEM_PS;
            alloc.size       = pg_count * MEM_PS;
            mutex_ulock(&pmm_lock);
            prtrace_end(
                "mem_ppalloc",
                "SUCCESS",
                "header_off=%p, padr=%p, size=%lu, fpg_idx=%lu, lpg_idx=%lu",
                alloc.header_off,
                alloc.padr,
                alloc.size,
                fpg_idx,
                lpg_idx
            );
            return alloc;
          }
          // look for next clear bit

          // first check current u64 for another clear page
          ffs = FFS(~(bitmap[pg_idx / 64] | (((uint64_t)1 << pg_idx) - 1)));

          // if no other clear page is found, check for next u64s
          // using fancy nscasq offsets
          if (ffs == -1) {
            first_found_idx =
                bitmap_size / 8 - as_nscasq(
                                      (uint64_t)(bitmap + pg_idx / 64 + 1),
                                      UINT64_MAX,
                                      bitmap_size / 8 - (pg_idx / 64 + 1)
                                  );

            if (bitmap[first_found_idx] == UINT64_MAX) {
              break;  // no more clear pages in this entry,
                      // go to the next one(next for loop)
            }

            ffs    = FFS(~bitmap[first_found_idx]);
            pg_idx = 64 * first_found_idx + ffs;
          } else {
            pg_idx = ALIGN_DN(pg_idx, 64) + ffs;
          }
        }
      }
    }
    pmm_header_off += sizeof(mem_pseg_header) + bitmap_size;
  }

  mutex_ulock(&pmm_lock);
  prtrace_end("mem_ppalloc", "ERR_MEM_NO_PHY_SPACE", 0);
  alloc.error = ERR_MEM_NO_PHY_SPACE;
  return alloc;
}

void mem_ppfree(void *pheader, mem_pallocation alloc) {
  prtrace_begin(
      "mem_ppfree",
      "pheader=%p, alloc={header_off=%p, padr=%p, size=%lu}",
      pheader,
      alloc.header_off,
      alloc.padr,
      alloc.size
  );

  mutex_lock(&pmm_lock);
  uint64_t *bitmap = (uint64_t *)(alloc.header_off + 1);
  size_t    fpg_idx =
      (size_t)(alloc.padr -
               ((mem_pseg_header *)(pheader + alloc.header_off))->padr) /
      MEM_PS;
  size_t pg_count = ALIGN_UP(alloc.size, MEM_PS) / MEM_PS;
  size_t lpg_idx  = fpg_idx + pg_count - 1;
  if (fpg_idx - lpg_idx < 64) {  // only one u64 to change
    bitmap[fpg_idx / 64] &= ~BITRANGE(fpg_idx % 64, lpg_idx % 64 + 1);
  } else {  // multiple u64s to set
    bitmap[fpg_idx / 64] &= ~BITRANGE(fpg_idx % 64, 64);
    bitmap[lpg_idx / 64] &= ~BITRANGE(0, lpg_idx % 64 + 1);
    size_t cpg_idx = ALIGN_UP(fpg_idx, 64);
    while (cpg_idx < lpg_idx + 64) {
      bitmap[cpg_idx / 64] = 0;
      cpg_idx += 64;
    }
  }
  mutex_ulock(&pmm_lock);

  prtrace_end("mem_ppfree", 0, 0);
}
