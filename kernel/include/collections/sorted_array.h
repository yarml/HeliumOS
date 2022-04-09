#ifndef HELIUM_COL_SA_H
#define HELIUM_COL_SA_H

#include <stddef.h>

// namespace col_sa

typedef int(*fpt_diff)(void const*, void const*);

struct col_sa
{
    void*    heap     ;
    fpt_diff fdiff    ;
    size_t   entry_len;
    size_t   size     ;
    size_t   cap      ;
};
typedef struct col_sa sorted_array;

#define COL_SA_EOSA(sa) (sa->heap + sa->entry_len * sa->size)

void col_sa_init(sorted_array* sa, void* heap, fpt_diff fdiff, size_t entry_len, size_t cap);

size_t col_sa_pins(sorted_array      * sa, void const* entry);

#define col_sa_vins(sa, v) ({ typeof(v) m_col_sa_tmp_val = v; col_sa_pins(sa, &m_col_sa_tmp_val); })

void col_sa_vdel(sorted_array      * sa, void const* entry, void* save);
void col_sa_idel(sorted_array      * sa, size_t index     , void* save);
void* col_sa_get(sorted_array const* sa, size_t index     , void* save);

#endif