#pragma once

#include <capi/types.hpp>
#include <stddef.h>
#include "priv.hpp"

namespace capi
{
    class mem_interface
    {
    public:
        /* Format
        /// Description
        virtual return_t f(args); // needed flags
        */
        /// Allocates a continuous n block of size MEMORY_UNIT_SIZES[type]
        virtual blockidx alloc_mem_block(size_t type, blockidx n) = 0; // MEM_ALLOC_BLOCK
        /// Free a continuous n block of size MEMORY_UNIT_SIZES[type]
        virtual void free_mem_block(size_t type, blockidx n) = 0; // MEM_FREE_BLOCK
        /// Get available non allocated memory size in bytes
        virtual mem_size memory_size() = 0; // MEM_SIZE;
        /// Set memory block of type type privilege level
        virtual void set_mem_block_perm(size_t type, blockidx idx, priv lvl) = 0; // MEM_BLOCK_PERM_W
        /// Reads memory block of type type privilege level
        virtual priv get_mem_block_perm(size_t type, blockidx idx) = 0; // MEM_BLOCK_PERM_R
    };
}