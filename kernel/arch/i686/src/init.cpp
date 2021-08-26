#include <i686.hpp>
#include <capi_core.hpp>

#include <capi/i686_architecture.hpp>
#include <memory.hpp>

namespace i686
{
    // TODO: If i686_arch ever needs a constructor
    // then it is better if we provide a init() function
    // as the constructor may not be called yet when this variable is used
    i686_arch arch;
    extern "C" void load_arch()
    {
        i686::mem::gdt::init();
        core::init(&arch);
    }
}