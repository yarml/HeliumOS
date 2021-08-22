#include <i686.hpp>
#include <capi_core.hpp>

namespace i686
{
    extern "C" void load_arch()
    {
        core::init();
    }
}