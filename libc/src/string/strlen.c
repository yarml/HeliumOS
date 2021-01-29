#include <string.h>
#include <stdint.h>

size_t strlen(const char* str)
{
    int size = 0;
    for(; *str != 0; str++, size++);
    return size;
}
