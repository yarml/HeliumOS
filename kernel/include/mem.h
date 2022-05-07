#ifndef HELIUM_MEM_H
#define HELIUM_MEM_H

#include <attributes.h>

typedef struct
{
    void*  padr;
    size_t size;
} pack mem_pmm_region_header;

void mem_init();

// Page size
#define MEM_PS (4096)

#endif