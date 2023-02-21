#ifndef HELIUM_INT_VCACHE_H
#define HELIUM_INT_VCACHE_H

#include <attributes.h>
#include <stdint.h>

#include <arch/mem.h>

/* Implementation reference:
    - PDEs store two fields in their metadata:
      - `lazy` 7 bits: Count of lazy PTEs
      - `free` 9 bits: Count of free PTEs
    - PTEs store one field in their metadata:
      - `age` 11 bits: when 0, it means the page is free
                       otherwise, it is an integer number
                       that counts how many times that PTE
                       has been lazy
 */

unused
static uint16_t pde_free_pages(mem_pde_ref *pde)
{
  uint16_t meta = mem_vpstruct_ptr_meta(pde);
  return meta & 0x3ff;
}

unused
static uint16_t pde_lazy_pages(mem_pde_ref *pde)
{
  return mem_vpstruct_ptr_meta(pde) >> 10 & 0x3f;
}

unused
static void pde_set_free(mem_pde_ref *pde, uint16_t free)
{
  uint16_t meta = mem_vpstruct_ptr_meta(pde);
  mem_vpstruct_ptr_set_meta(pde, (meta & 0xfc00) | (free & 0x3ff));
}

unused
static void pde_set_lazy(mem_pde_ref *pde, uint16_t lazy)
{
  uint16_t meta = mem_vpstruct_ptr_meta(pde);
  mem_vpstruct_ptr_set_meta(pde, (meta & 0x03ff) | (lazy & 0x3f) << 10);
}

unused
static uint16_t pte_age(mem_pte *pte)
{
  return mem_vpstruct2_meta(pte);
}

unused
static void pte_set_age(mem_pte *pte, uint16_t age)
{
  mem_vpstruct2_set_meta(pte, age);
}

#endif