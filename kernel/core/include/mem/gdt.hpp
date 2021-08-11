#pragma once

#include <stdint.h>

namespace mem::gdt
{
    class gdt_entry
    {
    public:
        gdt_entry();
        gdt_entry(uint32_t base, uint32_t limit, bool rw, bool exec, bool code_data_seg, bool mode32, bool mul4K, uint8_t priv);
    private:
        uint16_t m_limit_0_15;
        uint16_t m_base_0_15;
        uint8_t m_base_16_23;
        bool m_accessed:1; // Set to zero, only used by CPU
        bool m_rw:1; // Read Write
        bool m_dc:1; // Set to zero
        bool m_exec:1; // Can be executed
        bool m_type:1; // 1 for code and data segments
        uint8_t m_priv:2;
        bool m_present:1; // Should be 1 for the segment to be valid
        uint8_t m_limit_16_19:4;
        uint8_t m_zero:2; // Zero
        bool m_mode:1; // 0: 16 bit mode, 1: 32 bit mode
        bool m_granularity:1; // if 1 multiplies the limit by 4Kib
        uint8_t m_base_24_31;
    }__attribute__((packed));
    class gdt_descriptor
    {
    public:
        gdt_descriptor();
        gdt_descriptor(uint16_t size, gdt_entry* offset);
    private:
        uint16_t m_size;
        gdt_entry* m_offset;
    }__attribute__((packed));

    void init();

    constexpr int GDT_SIZE = 3;

    extern gdt_entry entries[GDT_SIZE];
    extern gdt_descriptor gdtr;
}

extern "C" void load_gdt(mem::gdt::gdt_descriptor gdtr);
