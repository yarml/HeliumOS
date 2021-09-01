#include <i686.hpp>
#include <capi_core.hpp>
#include <capi/i686_architecture.hpp>
#include <memory.hpp>
#include <multiboot.hpp>
#include <utils/array.hpp>
#include <debug.hpp>

// Temporary debug functions
// TODO: Remove those when not needed anymore
void dbg_char(capi::io_interface const* io, char c)
{
    // bochs e9 hack
    io->write_byte(debug::BOCHS_DBG_PORT, c);
}
void dbg_str(capi::io_interface const* io, char const* str)
{
    for(size_t i = 0; str[i] != 0; ++i)
        dbg_char(io, str[i]);
}
// can output to any base <= 16
void dbg_uint(capi::io_interface const* io, size_t val, size_t base)
{
    constexpr utils::array<char, 16> NUMBERS = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    static utils::array<char, 32> buf;

    if(val == 0)
    {
        dbg_char(io, '0');
        return;
    }

    int i = 30;

    for(;val != 0; --i, val /= base)
        buf[i] = NUMBERS[val % base];

    dbg_str(io, buf.data() + i + 1);
}
void dbg_ptr(capi::io_interface const* io, utils::ptr p)
{
    dbg_uint(io, reinterpret_cast<size_t>(p), 16);
}

namespace i686
{
    // TODO: If i686_arch ever needs a constructor
    // then it is better if we provide a init() function
    // as the constructor may not be called yet when this variable is used
    i686_arch arch;
    extern "C" void load_arch(multiboot::info_structure* mbt_info, multiboot::u32 mbt_sig)
    {
        capi::io_interface const* io = arch.get_io_interface();
        if(mbt_sig != multiboot::BOOTLOADER_MAGIC)
        {
            dbg_str(io, "Invalid multiboot signature: ");
            dbg_uint(io, mbt_sig, 16);
            dbg_char(io, '\n');
            arch.halt();
        }
        i686::mem::init(&arch, mbt_info);
        core::init(&arch);
    }
}