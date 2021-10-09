#include <i686.hpp>
#include <capi/early_heap.hpp>
#include <capi/early_ordered_list.hpp>
#include <capi/memory.hpp>

namespace capi
{
    static std_early_heap s_sys_early_heap;
    static early_ordered_list<mem_boundary> s_mem_map;
    std_early_heap* system_early_heap()
    {
        return &s_sys_early_heap;
    }
    early_ordered_list<mem_boundary>& system_memory_map()
    {
        return s_mem_map;
    }
}
namespace i686
{
    void init()
    {
        capi::s_sys_early_heap = capi::std_early_heap();
        capi::s_mem_map = capi::early_ordered_list<capi::mem_boundary>(&capi::s_sys_early_heap);
    }
}

