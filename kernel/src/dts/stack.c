#include <stdlib.h>
#include <string.h>

#include <dts/stack.h>

dts_stack *dts_stack_create(size_t elen, size_t init_cap) {
  dts_stack *stack = calloc(1, sizeof(dts_stack));
  if (!stack) {
    return 0;
  }

  stack->elen = elen;
  stack->cap  = init_cap;

  if (init_cap) {
    stack->stack = calloc(init_cap, elen);
    if (!stack->stack) {
      free(stack);
      return 0;
    }
  }

  return stack;
}
void dts_stack_destroy(dts_stack *stack) {
  free(stack);
}

void dts_stack_push(dts_stack *stack, void *element) {
  if (stack->len == stack->cap) {
    dts_stack_resize(stack, stack->cap ? stack->cap * 2 : 4);
  }

  memcpy(stack->stack + stack->len * stack->elen, element, stack->elen);
  stack->len++;
}
void dts_stack_pop(dts_stack *stack, void *out_element) {
  // A little bit of bounds checking
  if (stack->len == 0) {
    if (out_element) {
      memset(out_element, 0, stack->elen);
    }
    return;
  }
  stack->len--;
  if (out_element) {
    memcpy(out_element, stack->stack + stack->len * stack->elen, stack->elen);
  }
}
void *dts_stack_at(dts_stack *stack, size_t index, void *out_element) {
  if (index >= stack->len) {
    if (out_element) {
      memset(out_element, 0, stack->elen);
    }
    return 0;
  }

  void *target = stack->stack + index * stack->elen;

  if (out_element) {
    memcpy(out_element, target, stack->elen);
  }

  return target;
}

void dts_stack_resize(dts_stack *stack, size_t newcap) {
  // If it fails, it failed :-)
  stack->stack = reallocarray(stack->stack, newcap, stack->elen);
}