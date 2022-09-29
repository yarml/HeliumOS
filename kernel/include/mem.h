#ifndef HELIUM_MEM_H
#define HELIUM_MEM_H

#include <attributes.h>
#include <stdbool.h>
#include <stddef.h> 
#include <error.h>

#include <arch/mem.h>

struct MEM_PSEG_HEADER
{
    void* padr;
    size_t size;
} pack;
typedef struct MEM_PSEG_HEADER mem_pseg_header;

struct MEM_PALLOCATION
{
    size_t header_off;
    void* padr;
    size_t size;
    errno_t error;
};
typedef struct MEM_PALLOCATION mem_pallocation;

void mem_init();

/* mem_p* */

mem_pallocation mem_ppalloc(void* pheader, size_t size, size_t alignement, bool cont, void* below);
void mem_ppfree(void* pheader, mem_pallocation alloc);

/* mem_v* */
errno_t mem_vmap(void* vadr, void* padr, size_t size, int flags);
errno_t mem_vumap(void* vadr, size_t size);


// ERR_MEM memory operations errors
#define ERR_MEM_ALN          (-1) /* Alignment error */
#define ERR_MEM_NO_PHY_SPACE (-2) /* No physical space */
#define ERR_MEM_NULL_SIZE    (-3) /* Allocation/mapping of size 0 */
#define ERR_MEM_INV_VADR     (-4) /* Invalid virtual address(eg. non canonical address on systems that require it) */

// MAPF memory mapping flags
#define MAPF_R   (1<<0) /* Read */
#define MAPF_W   (1<<1) /* Write */
#define MAPF_X   (1<<2) /* Execute */

#define MAPF_U   (1<<3) /* User access */

#define MAPF_P2M (1<<4) /* Map using 2 Mib page entries */
#define MAPF_P1G (1<<5) /* Map using 1 Gib page entries */

#define MAPF_G   (1<<6) /* Global page (pages above KVMSPACE are always global) */

#endif