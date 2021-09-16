#include <memory.hpp>
#include <debug.hpp>
#include <utils/maths.hpp>
#include <multiboot.h>
#include <i686.hpp>
#include <early_ordered_list.hpp>
#include <multiboot.hpp>

namespace i686::mem
{
    class mem_block_limit
    {
    public:
        enum class type
        {
            START,
            END
        };
    public:
        constexpr mem_block_limit(capi::adr limit, multiboot::mem_type priv, type t)
            : m_limit(limit),
              m_priv(priv),
              m_type(t)
        { }
    public:
        constexpr capi::adr limit()
        {
            return m_limit;
        }
        constexpr multiboot::mem_type priv()
        {
            return m_priv;
        }
        constexpr type type()
        {
            return m_type;
        }
        constexpr bool operator<(mem_block_limit const& other)
        {
            return m_limit < other.m_limit;
        }
    private:
        capi::adr           m_limit;
        multiboot::mem_type m_priv ;
        enum type           m_type ;
    };
    class mem_boundary
    {
    public:
        constexpr mem_boundary(capi::adr limit, multiboot::mem_type priv)
            : m_limit(limit),
              m_priv(priv)
        { }
    public:
        constexpr capi::adr limit()
        {
            return m_limit;
        }
        constexpr multiboot::mem_type priv()
        {
            return m_priv;
        }
    public:
        constexpr bool operator<(mem_boundary const& other)
        {
            return m_limit < other.m_limit;
        }
        constexpr bool operator!=(mem_boundary const& other)
        {
            return m_limit != other.m_limit || m_priv != other.m_priv;
        }
    private:
        capi::adr           m_limit;
        multiboot::mem_type m_priv ;
    };
    static void detect_memory(capi::architecture* arch, multiboot_info_t* mbt_info)
    {
        capi::io_interface const* io = arch->get_io_interface();
        if(!maths::check_bit(mbt_info->flags, 6))
        {
            dbg_str(io, "Invalid memory map!\n");
            arch->halt();
        }
        early_ordered_list<mem_block_limit> limits(i686::early_heap());
        size_t mem_map_len = 0; // length in bytes
        size_t mem_map_size = 0; // size in elements
        for(; mem_map_len < mbt_info->mmap_length;
            ++mem_map_size, mem_map_len +=
                reinterpret_cast<multiboot::mmap_entry*>
                (mbt_info->mmap_addr + mem_map_len)->size 
                + sizeof(
                    reinterpret_cast<multiboot::mmap_entry*>
                    (mbt_info->mmap_addr + mem_map_len)->size
                )/* since the size variable itself is not counted */)
        /* in layman's terms: for each element in the memory map */
        {
            multiboot::mmap_entry* current =
                reinterpret_cast<multiboot::mmap_entry*>
                (mbt_info->mmap_addr + mem_map_len);
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
        early_ordered_list<mem_boundary> mem_map(i686::early_heap());
        early_ordered_list<int> privs(i686::early_heap());
        // Very slim chances that the bootloader will throw us this number in the type field
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
            if(current->next == nullptr)
            {
                if(current->item.limit() == 0xFF'FF'FF'FF)
                    mem_map.remove_item(current->item);
                break;
            }
            if(current->item.limit() + 1 == current->next->item.limit())
                mem_map.remove_item(current->item);
        }
        dbg_str(io, "In total, the memory map has a size of: ");
        dbg_uint(io, mem_map_size, 10);
        dbg_str(io, " elements\n");
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
    void init(capi::architecture* arch, multiboot::info_structure* mbt_info)
    {
        detect_memory(arch, (multiboot_info_t*) mbt_info);
        gdt::init(arch);
    }
}