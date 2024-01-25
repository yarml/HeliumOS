#include <stdint.h>
#include <stdio.h>

#include <dts/stack.h>

#include "internal_kmod.h"

static dts_stack *modlist;

void kmod_init() {
  modlist = dts_stack_create(sizeof(uintptr_t), 0);
}

void i_addmod(kmod *mod) {
  dts_stack_push(modlist, &mod);
  printd("Added mod: %p\n", mod);
}

void kmod_post() {
  for (size_t i = 0; i < modlist->len; ++i) {
    kmod *mod = *(kmod **)dts_stack_at(modlist, i, 0);
    if (mod->functable.post) {
      mod->functable.post();
    }
  }
}