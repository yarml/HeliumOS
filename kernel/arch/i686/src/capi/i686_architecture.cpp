#include <capi/i686_architecture.hpp>

namespace i686
{
   
    capi::mem_interface* i686_arch::get_mem_interface()
    {
        return nullptr;
    }
    capi::io_interface* i686_arch::get_io_interface()
    {
        return &m_io;
    }
    capi::proc_interface* i686_arch::get_proc_interface()
    {
        return nullptr;
    }
    capi::interrupt_interface* i686_arch::get_interrupt_interface()
    {
        return nullptr;
    }
}