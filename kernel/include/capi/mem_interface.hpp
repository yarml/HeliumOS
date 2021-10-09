#pragma once

#include <capi/types.hpp>
#include <stddef.h>
#include "priv.hpp"

namespace capi
{
    class mem_interface
    {
    public:
		/// Allocates a new physical block and returns its unique index
		virtual pblockidx alloc_physical_block() const = 0;
		/// Frees the physical block with the specified index
		virtual void free_physical_vlock(pblockidx) const = 0;
		/// Maps a virtual block to a physical one
		virtual void map_virtual_block(pblockidx, vblockidx) const = 0;
		/// Unmaps the specified virtual block
		virtual void unmap_virtual_block(vblockidx) const = 0;
    };
}
