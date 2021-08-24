#include <capi/i686_io_interface.hpp>

extern "C" void i686_outb(capi::ioadr port, capi::byte data);
extern "C" void i686_outw(capi::ioadr port, capi::word data);
extern "C" void i686_outdw(capi::ioadr port, capi::dword data);

extern "C" capi::byte i686_inb(capi::ioadr port);
extern "C" capi::word i686_inw(capi::ioadr port);
extern "C" capi::dword i686_indw(capi::ioadr port);

namespace i686
{

    void i686_io_interface::write_byte(capi::ioadr a, capi::byte o)
    {
        i686_outb(a, o);
    }
    void i686_io_interface::write_word(capi::ioadr a, capi::word o)
    {
        i686_outw(a, o);
    }
    void i686_io_interface::write_dword(capi::ioadr a, capi::dword o)
    {
        i686_outdw(a, o);
    }
    void i686_io_interface::write_qword(capi::ioadr a, capi::qword o)
    {
        // equivalent to write_dword in x86
        i686_outdw(a, o);
    }

    capi::byte i686_io_interface::read_byte(capi::ioadr a)
    {
        return i686_inb(a);
    }
    capi::word i686_io_interface::read_word(capi::ioadr a)
    {
        return i686_inw(a);
    }
    capi::dword i686_io_interface::read_dword(capi::ioadr a)
    {
        return i686_indw(a);
    }
    capi::qword i686_io_interface::read_qword(capi::ioadr a)
    {
        // equivalent to read_dword in x86
        return i686_indw(a);
    }
}
