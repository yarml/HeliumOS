#include "vcache.h"

size_t i_vcp_bitmap[VCACHE_PAGES * 2 / sizeof(size_t)];
vcache_unit *i_vclp_table[VCACHE_LAZY_PAGES];

vcache_unit vcache_map(void *padr)
{

}

void vcache_lumap(vcache_unit unit)
{

}

void vcache_umap(vcache_unit unit)
{

}

void vcache_flush()
{
    
}
