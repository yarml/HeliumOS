#ifndef HELIUM_DTS_STACK_H
#define HELIUM_DTS_STACK_H

#include <stddef.h>

typedef struct DTS_STACK {
  void  *stack;
  size_t elen;
  size_t len;
  size_t cap;
} dts_stack;

dts_stack *dts_stack_create(size_t elen, size_t init_cap);
void       dts_stack_destroy(dts_stack *stack);

void  dts_stack_push(dts_stack *stack, void *element);
void  dts_stack_pop(dts_stack *stack, void *out_element);
void *dts_stack_at(dts_stack *stack, size_t index, void *out_element);

void dts_stack_resize(dts_stack *stack, size_t newcap);

#endif