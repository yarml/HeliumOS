#include <memory.hpp>
#include <debug.hpp>
#include <utils/maths.hpp>
#include <multiboot.h>

namespace i686::mem
{
    static void detect_memory(capi::architecture* arch, multiboot_info_t* mbt_info)
    {
        dbg_str(arch->get_io_interface(), "Detecting memory\n");
        dbg_uint(arch->get_io_interface(), mbt_info->flags, 2);
        dbg_char(arch->get_io_interface(), '\n');
        dbg_uint(arch->get_io_interface(), mbt_info->mmap_addr, 16);
        dbg_char(arch->get_io_interface(), '\n');
        if(!maths::check_bit(mbt_info->flags, 6))
        {
            dbg_str(arch->get_io_interface(), "Invalid memory map!\n");
            arch->halt();
        }
        capi::io_interface const* io = arch->get_io_interface();
        size_t mem_map_len = 0; // length in bytes
        size_t mem_map_size = 0; // size in elements
        dbg_str(io, "-------------------\n");
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
            // size
            dbg_str(io, "Size: ");
            dbg_uint(io, current->size + sizeof(
                    reinterpret_cast<multiboot::mmap_entry*>
                    (mbt_info->mmap_addr + mem_map_len)->size
                ), 10);
            dbg_char(io, '\n');
            // base
            dbg_str(io, "Base: ");
            dbg_uint(io, current->addr_low, 16);
            dbg_char(io, '\n');
            // len
            dbg_str(io, "Len: ");
            dbg_uint(io, current->len_low, 16);
            dbg_char(io, '\n');
            // type
            dbg_str(io, "Type: ");
            dbg_uint(io, (uint32_t) current->type, 10);
            dbg_char(io, '\n');
            dbg_str(io, "-------------------\n");
        }
        dbg_str(io, "In total, the memory map has a size of: ");
        dbg_uint(io, mem_map_size, 10);
        dbg_str(io, " elements\n");
    }
    void init(capi::architecture* arch, multiboot::info_structure* mbt_info)
    {
        detect_memory(arch, (multiboot_info_t*) mbt_info);
        gdt::init(arch);
    }
}