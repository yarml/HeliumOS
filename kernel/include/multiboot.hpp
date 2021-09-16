#pragma once

#include <support/attributes.hpp>
#include <stdint.h>

namespace multiboot
{
    using u8  = uint8_t ;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    
    constexpr u32 BOOTLOADER_MAGIC = 0x2BADB002;

    struct aout_symbol_table
    {
        u32 tabsize;
        u32 strsize;
        u32 addr;
        u32 reserved;
    };
    struct elf_section_header_table
    {
        u32 num;
        u32 size;
        u32 addr;
        u32 shndx;
    };
    enum class framebuf_type : u8
    {
        INDEXED = 0,
        RGB = 1,
        EGA_TEXT = 2
    };
    struct info_structure
    {
        u32 flags;

        u32 mem_lower;
        u32 mem_upper;

        u32 boot_device;

        u32 cmd_line;

        u32 mod_count;
        u32 mod_adr;

        union
        {
            aout_symbol_table aout_sym;
            elf_section_header_table elf_sec;
        };

        u32 mmap_len;
        u32 mmap_adr;

        u32 drive_len;
        u32 drive_adr;

        u32 config_table;

        u32 bootloader;

        u32 apm_table;

        u32 vbe_control_info;
        u32 vbe_mode_info;
        u16 vbe_mode;
        u16 vbe_interface_seg;
        u16 vbe_interface_off;
        u16 vbe_interface_len;

        u64 framebuffer_addr;
        u32 framebuffer_pitch;
        u32 framebuffer_width;
        u32 framebuffer_height;
        u8 framebuffer_bpp;
        framebuf_type framebuffer_type;
        
        union
        {
            struct
            {
                u32 framebuffer_palette_addr;
                u16 framebuffer_palette_num_colors;
            };
            struct
            {
                u8 framebuffer_red_field_position;
                u8 framebuffer_red_mask_size;
                u8 framebuffer_green_field_position;
                u8 framebuffer_green_mask_size;
                u8 framebuffer_blue_field_position;
                u8 framebuffer_blue_mask_size;
            };
        };
    };
    enum class mem_type : u32
    {
        AVAILABLE = 1,
        RESERVED = 2,
        ACPI = 3,
        NVS = 4,
        BADRAM = 5
    };
    struct mmap_entry
    {
        u32 size;
        u32 addr_low;
        u32 addr_high;
        u32 len_low;
        u32 len_high;
        mem_type type;
    } PACKED;
}