#include <stddef.h>
#include <stdint.h>
#include <utils.h>

#include <asm/movs.h>

#include "../internal_string.h"

void *memcpy(void *to, void const *from, size_t size)
{
  void *oto = to;

  // TODO: Maybe max_alignment can be calculated in constant time
  size_t max_alignment = 1;

  if((uintptr_t) to % 8 == (uintptr_t)from % 8)
    max_alignment = 8;
  else if((uintptr_t)to % 4 == (uintptr_t)from % 4)
    max_alignment = 4;
  else if((uintptr_t)to % 2 == (uintptr_t)from % 2)
    max_alignment = 2;

  // copy individual bytes until both from and to are aligned to max_alignment
  // at worst case, it should individually copy 7 bytes
  while(
    (
        (uintptr_t)to % max_alignment
      || (uintptr_t)from % max_alignment
    )
    && size
  ) {
    *(uint8_t *)to++ = *(uint8_t*)from++;
    --size;
  }
  // Mass copy the now aligned bytes if there exist enough of them for mass
  // copying to be efficient
  if(size >= MASS_OP_MIN_BYTES)
  {
    void(*movsfp)(uint64_t, uint64_t, uint64_t);
    switch(max_alignment)
    {
    case 8:
      movsfp = as_movsq;
      break;
    case 4:
      movsfp = as_movsd;
      break;
    case 2:
      movsfp = as_movsw;
      break;
    default:
      movsfp = as_movsb;
      break;
    }
    movsfp((uint64_t)to, (uint64_t)from, size / max_alignment);
    to += ALIGN_DN(size, max_alignment);
    from += ALIGN_DN(size, max_alignment);
    size -= ALIGN_DN(size, max_alignment);
  }
  // Copy leftover bytes, should be a maximum of 7
  as_movsb((uint64_t)to, (uint64_t)from, size);

  return oto;
}