#include <i686.hpp>
#include <capi_core.hpp>

#include <capi/i686_architecture.hpp>

namespace i686
{
    extern "C" void load_arch()
    {
        i686_arch arch;
        core::init(&arch);
    }
}