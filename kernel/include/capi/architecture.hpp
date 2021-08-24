#pragma once

#include "mem_interface.hpp"
#include "io_interface.hpp"
#include "proc_interface.hpp"
#include "interrupt_interface.hpp"

namespace capi
{
    class architecture
    {
    public:
        virtual mem_interface* get_mem_interface() = 0;
        virtual io_interface* get_io_interface() = 0;
        virtual proc_interface* get_proc_interface() = 0;
        virtual interrupt_interface* get_interrupt_interface() = 0;
    };
}

