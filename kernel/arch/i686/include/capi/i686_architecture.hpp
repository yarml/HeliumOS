#pragma once

#include <capi/architecture.hpp>
#include "i686_io_interface.hpp"

namespace i686
{
    class i686_arch : public capi::architecture
    {
    public:
        capi::mem_interface* get_mem_interface() override;
        capi::io_interface* get_io_interface() override;
        capi::proc_interface* get_proc_interface() override;
        capi::interrupt_interface* get_interrupt_interface() override;
    private:
        i686_io_interface m_io;
    };
}
