#include <errors.h>
#include <stdio.h>
#include <debug.h>

void error_physical_memory_alloc(char const* detail)
{
    printf("\nPhysical memory allocation error: %s\n", detail);
    LOOP;
}
