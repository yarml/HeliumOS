#include <arch/mem.h>

uint16_t mem_vpstruct_ptr_meta(mem_vpstruct_ptr *s)
{
  return s->free0 | s->free1 << 1 | s->free2 << 5;
}
void mem_vpstruct_ptr_set_meta(mem_vpstruct_ptr *s, uint16_t meta)
{
  s->free0 = meta & 1;
  s->free1 = meta >> 1 & 0x1f;
  s->free2 = meta >> 5 & 0x7ff;
}

uint16_t mem_vpstruct_meta(mem_vpstruct *s)
{
  return s->free0 | s->free1 >> 3;
}
void mem_vpstruct_set_meta(mem_vpstruct *s, uint16_t meta)
{
  s->free0 = meta & 0x7;
  s->free1 = meta >> 3 & 0x7f;
}

uint16_t mem_vpstruct2_meta(mem_vpstruct2 *s)
{
  return s->free0 | s->free1 >> 3;
}
void mem_vpstruct2_set_meta(mem_vpstruct2 *s, uint16_t meta)
{
  s->free0 = meta & 0x7;
  s->free1 = meta >> 3 & 0x7f;
}
