#include <mem/gdt.hpp>


namespace mem::gdt
{
    gdt_entry::gdt_entry(uint32_t base, uint32_t limit, bool rw, bool exec, bool code_data_seg, bool mode32, bool mul4K, uint8_t priv)
    {
        m_limit_0_15 = limit & 0x0000FFFF;
        m_limit_16_19 = (limit & 0x000F0000) >> 16;
        m_base_0_15 = base  & 0x0000FFFF;
        m_base_16_23 = (base  & 0x00FF0000) >> 16;
        m_base_24_31 = (base  & 0xFF000000) >> 24;
        m_accessed = false;
        m_rw = rw;
        m_dc = false;
        m_exec = exec;
        m_type = code_data_seg;
        m_priv = priv;
        m_present = true;
        m_zero = 0;
        m_mode = mode32;
        m_granularity = mul4K;
    }
    gdt_entry::gdt_entry() { }

    gdt_descriptor::gdt_descriptor(uint16_t size, gdt_entry* offset)
    {
        m_size = size;
        m_offset = offset;
    }
    gdt_descriptor::gdt_descriptor() { }


    gdt_entry entries[GDT_SIZE];
    gdt_descriptor gdtr;

    void init()
    {
        entries[0] = gdt_entry(0, 0, false, false, false, false, false, 0);
        entries[1] = gdt_entry(0, 0xFFFFFFFF, true, true, true, true, true, 0);
        entries[2] = gdt_entry(0, 0xFFFFFFFF, true, false, true, true, true, 0);

        gdtr = gdt_descriptor(sizeof(entries), entries);

        load_gdt(gdtr);
    }
}
