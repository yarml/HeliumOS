#ifndef HELIUM_ERRORS_H
#define HELIUM_ERRORS_H


void error_physical_memory_alloc(char const* detail);

void error_heap_alloc(char const* detail);

void error_virtual_memory_alloc(char const* detail);

#endif