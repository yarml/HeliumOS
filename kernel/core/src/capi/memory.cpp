#include <capi/memory.hpp>
#include <utils/maths.hpp>
#include <capi/early_ordered_list.hpp>
#include <capi/early_heap.hpp>
#include <debug.hpp>

namespace capi
{

    static void print_memory_map(capi::io_interface const* io, early_ordered_list<mem_boundary>& mem_map)
    {
        dbg_str(io, "Printing memory map\n");
        dbg_str(io, "-------------------------------------\n");
        for(auto current = mem_map.head(); current != nullptr; current = current->next)
        {
            dbg_str(io, "Limit: ");
            dbg_uint(io, current->item.limit(), 16);
            dbg_str(io, ", priv: ");
            dbg_uint(io, static_cast<int>(current->item.priv()), 16);
            dbg_char(io, '\n');
        }
        dbg_str(io, "done\n");
    }

    void detect_memory(capi::architecture* arch, multiboot::info_structure* mbt_info)
    {
        capi::io_interface const* io = arch->get_io_interface();
        if(!maths::check_bit(mbt_info->flags, 6))
        {
            dbg_str(io, "Invalid memory map!\n");
            arch->halt();
        }
        
        early_ordered_list<mem_block_limit> limits(capi::system_early_heap());
        size_t mem_map_len = 0; // length in bytes
        size_t mem_map_size = 0; // size in elements
        for(; mem_map_len < mbt_info->mmap_len;
            ++mem_map_size, mem_map_len +=
                reinterpret_cast<multiboot::mmap_entry*>
                (mbt_info->mmap_adr + mem_map_len)->size 
                + sizeof(
                    reinterpret_cast<multiboot::mmap_entry*>
                    (mbt_info->mmap_adr + mem_map_len)->size
                )/* since the size variable itself is not counted */)
        /* in layman's terms: for each element in the memory map */
        {
            multiboot::mmap_entry* current =
                reinterpret_cast<multiboot::mmap_entry*>
                (mbt_info->mmap_adr + mem_map_len);
            // For the i686 architecture we can ignore the high address
            limits.add_item(
                mem_block_limit(
                    current->addr_low,
                    current->type,
                    mem_block_limit::type::START
                )
            );
            limits.add_item(
                mem_block_limit(
                    current->addr_low + current->len_low - 1,
                    current->type,
                    mem_block_limit::type::END
                )
            );
        }

        early_ordered_list<mem_boundary>& mem_map = capi::system_memory_map();
        early_ordered_list<int> privs(capi::system_early_heap());

        for(auto current = limits.head(); current != nullptr; current = current->next)
        {
            int* old_top_ptr = privs.top();
            int old_top = old_top_ptr == nullptr ? 0xFF'FF'FF'FF : *old_top_ptr;
            if(current->item.type() == mem_block_limit::type::START)
                privs.add_item(static_cast<int>(current->item.priv()));
            else
                privs.remove_item(static_cast<int>(current->item.priv()));
            int* current_top_ptr = privs.top();
            int current_top = current_top_ptr == nullptr ? 0xFF'FF'FF'FF : *current_top_ptr;
            if(old_top != current_top)
                mem_map.add_item(
                    mem_boundary(
                        current->item.limit(), 
                        static_cast<multiboot::mem_type>(current_top)
                    )
                );
        }
        for(auto current = mem_map.head(); current != nullptr; current = current->next)
        {
            if(current->item.limit() + 1 == current->next->item.limit())
                mem_map.remove_item(current->item);
        }
        dbg_str(io, "In total, the memory map has a size of: ");
        dbg_uint(io, mem_map_size, 10);
        dbg_str(io, " elements\n");
        print_memory_map(io, mem_map);
    }
    
    utils::ptr alloc_physical_memory(utils::size_type size)
    {
        auto& mem_map = system_memory_map();
        for(auto current = mem_map.head();
            current != nullptr;
            current = current->next)
        {
            if(
                    current->item.priv() != multiboot::mem_type::AVAILABLE 
                || (current->next != nullptr ? 
                    (current->item.limit() + size > current->next->item.limit()) 
                : true)
                /* in layman's terms: if not available,
                 or available but small compared to the wanted size */
            )
                continue;
            current->item.set_priv(multiboot::mem_type::ALLOCATED);
            if(current->next != nullptr)
                if(current->next->item.limit() != current->item.limit() + size)
                    mem_map.add_item(
                        mem_boundary(
                            current->item.limit() + size,
                            multiboot::mem_type::AVAILABLE
                        )
                    );
            return reinterpret_cast<utils::ptr>(current->item.limit());
        }
        return nullptr;
    }

    void free_physical_memory(utils::ptr ptr)
    {
        auto& mem_map = system_memory_map();
        capi::early_ordered_list<mem_boundary>::list_item* previous = nullptr;
        for(auto current = mem_map.head();
            current != nullptr;
            current = current->next)
        {
            if(current->item.limit() == reinterpret_cast<capi::adr>(ptr))
            {
                current->item.set_priv(multiboot::mem_type::AVAILABLE);
                if(current->next != nullptr)
                    if(current->next->item.priv() == multiboot::mem_type::AVAILABLE)
                        mem_map.remove_from_head(current->next, current);
                if(previous != nullptr)
                    if(previous->item.priv() == multiboot::mem_type::AVAILABLE)
                        mem_map.remove_from_head(current, previous);
                return;
            }
            previous = current;
        }
    }

}
