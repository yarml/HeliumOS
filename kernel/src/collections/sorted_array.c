#include <collections/sorted_array.h>

#include <string.h>

void col_sa_init(sorted_array* sa, void* heap, fpt_diff fdiff, size_t entry_len, size_t cap)
{
    sa->heap      = heap     ;
    sa->fdiff     = fdiff    ;
    sa->entry_len = entry_len;
    sa->size      = 0        ;
    sa->cap       = cap      ;
}

// TODO: do a binary search here instead
size_t col_sa_pins(sorted_array* sa, void const* entry)
{
    if(sa->cap && sa->size >= sa->cap)
        return -1;
    void* next = sa->heap;
    // if elements are equal, just continue, so that we would move less data on the next step
    while(next != COL_SA_EOSA(sa) && sa->fdiff(entry, next) >= 0) 
        next += sa->entry_len;
    memmove(next + sa->entry_len, next, sa->size * sa->entry_len - (next - sa->heap));
    memcpy(next, entry, sa->entry_len);
    ++sa->size;
    return (next - sa->heap) / sa->entry_len;
}

// TODO: binary search here too
void col_sa_vdel(sorted_array* sa, void const* entry, void* save)
{
    void* next = sa->heap;
    while(sa->fdiff(entry, next) && next != COL_SA_EOSA(sa))
        next += sa->entry_len;
    if(save)
        memcpy(save, next, sa->entry_len);
    memmove(next, next + sa->entry_len, sa->size * sa->entry_len - (next + sa->entry_len - sa->heap));
    --sa->size;
}
void col_sa_idel(sorted_array* sa, size_t index, void* save)
{
    if(index < sa->size)
    {
        void* next = sa->heap + index * sa->entry_len;
        if(save)
            memcpy(save, next, sa->entry_len);
        memmove(next, next + sa->entry_len, sa->size * sa->entry_len - (next + sa->entry_len - sa->heap));
        --sa->size;
    }
}

void* col_sa_get(sorted_array const* sa, size_t index, void* save)
{
    if(index < sa->size)
    {
        void* next = sa->heap + index * sa->entry_len;
        if(save)
            memcpy(save, next, sa->entry_len);
        return next;
    }
    else
        return 0;
}

