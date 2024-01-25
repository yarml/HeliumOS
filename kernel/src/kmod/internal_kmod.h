#ifndef HELIUM_INT_KMOD_H
#define HELIUM_INT_KMOD_H

#include <hashtable.h>
#include <kmod.h>

#include <dts/stack.h>

extern hash_table *i_ksym_table;

void i_addmod(kmod *mod);

#endif