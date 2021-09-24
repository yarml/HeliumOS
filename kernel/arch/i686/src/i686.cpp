#include <i686.hpp>
#include <capi/early_heap.hpp>


namespace capi
{
    static std_early_heap s_sys_early_heap;
    std_early_heap* system_early_heap()
    {
        return &s_sys_early_heap;
    }
}
namespace i686
{
    void init()
    {
        capi::s_sys_early_heap = capi::std_early_heap();
    }
}

