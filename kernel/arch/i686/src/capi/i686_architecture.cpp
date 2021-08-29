#include <capi/i686_architecture.hpp>
#include <support/attributes.hpp>

namespace i686
{
    extern "C" NORET void i686_halt();
    capi::mem_interface const* i686_arch::get_mem_interface() const
    {
        return nullptr;
    }
    capi::io_interface const* i686_arch::get_io_interface() const
    {
        return &m_io;
    }
    capi::proc_interface const* i686_arch::get_proc_interface() const
    {
        return nullptr;
    }
    capi::interrupt_interface const* i686_arch::get_interrupt_interface() const
    {
        return nullptr;
    }
    void i686_arch::halt() const
    {
        i686_halt();
    }
}