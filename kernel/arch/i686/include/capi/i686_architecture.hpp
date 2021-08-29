#pragma once

#include <capi/architecture.hpp>
#include "i686_io_interface.hpp"

namespace i686
{
    class i686_arch : public capi::architecture
    {
    public:
        capi::mem_interface const* get_mem_interface() const override;
        capi::io_interface const* get_io_interface() const override;
        capi::proc_interface const* get_proc_interface() const override;
        capi::interrupt_interface const* get_interrupt_interface() const override;
        void halt() const override;
    private:
        i686_io_interface m_io;
    };
}
