#include <string.h>

void* memcpy(void* to, const void* from, size_t size)
{
    for(size_t i = 0; i < size; i++)
        ((char*)to)[i] = ((char*)from)[i];
    return to;
}