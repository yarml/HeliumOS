#include <i686.hpp>


namespace i686
{
    static mem::std_early_heap s_heap;

    mem::std_early_heap* early_heap()
    {
        return &s_heap;
    }

    void init()
    {
        s_heap = mem::std_early_heap();
    }
}
