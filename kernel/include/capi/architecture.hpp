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
        virtual mem_interface const* get_mem_interface() const = 0;
        virtual io_interface const* get_io_interface() const = 0;
        virtual proc_interface const* get_proc_interface() const = 0;
        virtual interrupt_interface const* get_interrupt_interface() const = 0;
        virtual void halt() const = 0;
    };
}

