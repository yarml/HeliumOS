#ifndef HELIUM_MEM_H
#define HELIUM_MEM_H

#include <attributes.h>
#include <stdbool.h>

typedef struct
{
    void* padr;
    size_t size;
} pack mem_pseg_header;

typedef struct
{
    mem_pseg_header* header;
    void* padr;
    size_t size;
} mem_pallocation;

void mem_init();

/* mem_pp* */

mem_pallocation mem_ppalloc(void* pheader, size_t size, bool cont, void* below);
void mem_ppfree(mem_pallocation alloc);

// Page size
#define MEM_PS (4096)

#endif