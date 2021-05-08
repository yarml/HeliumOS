#include <memory/heap.hpp>
#include <stddef.h>

void* operator new(size_t size)
{
    return memory::heap::knew(size);
}
 
void* operator new[](size_t size)
{
    return memory::heap::knew(size);
}
 
void operator delete(void* p)
{
    memory::heap::kfree(p);
}
 
void operator delete[](void* p)
{
    memory::heap::kfree(p);
}

void operator delete(void* p, long unsigned int)
{
    memory::heap::kfree(p);
}

void operator delete [](void* p, long unsigned int)
{
    memory::heap::kfree(p);
}