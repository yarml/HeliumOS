#include <capi_core.hpp>
#include <capi/types.hpp>
#include <capi/consts.hpp>
#include <debug.hpp>

namespace core
{
    void init(capi::architecture* arch)
    {
        dbg_str(arch->get_io_interface(), "Core kernel loaded\n");
    }
}
