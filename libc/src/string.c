#include <stddef.h>
#include <stdint.h>

size_t strlen(const char* str)
{
    int size = 0;
    for(; *str != 0; str++, size++);
    return size;
}

char* strchr(const char* string, int c)
{
    uint32_t size = strlen(string);
    for(uint32_t i = 0; i < size; i++)
        if(string[i] == c)
            return (char*) &(string[i]);
    return NULL;
}

void* memcpy(void* to, const void* from, size_t size)
{
    for(size_t i = 0; i < size; i++)
        ((char*)to)[i] = ((char*)from)[i];
    return to;
}

uint32_t chrpos(const char* str, char c)
{
    uint32_t size = strlen(str);
    for(uint32_t i = 0; i < size; i++)
        if(str[i] == c)
            return i;
    return -1;
}

int strcmp(const char* s1, const char* s2)
{
    int32_t size1 = strlen(s1);
    int32_t size2 = strlen(s2);
    int32_t size = size1 > size2 ? size2 : size1;
    for(int32_t i = 0; i < size; i++)
        if(s1[i] != s2[i])
            return s1[i] - s2[i];
    return size1 - size2;
}

char* strcpy(char* to, const char* from)
{
    memcpy(to, from, strlen(from) + 1);
    return to;
}