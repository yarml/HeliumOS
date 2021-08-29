#include <memory.hpp>
#include <utils/array.hpp>
#include <debug.hpp>

namespace i686::mem
{
    namespace gdt
    {
        segment_descriptor::segment_descriptor() { }
        segment_descriptor::segment_descriptor(
                capi::adr base,
                capi::mem_size limit, 
                capi::byte dpl, 
                bool present, 
                bool limit_4kib, 
                bool code_data, 
                segment_type type
            )
            : m_limit_15_00(limit & 0x00'00'FF'FF),
              m_base_15_00(base & 0x00'00'FF'FF),
              m_base_23_16((base & 0x00'FF'00'00) >> 16),
              m_segment_type(type & 0x0F),
              m_code_data(code_data),
              m_dpl(dpl),
              m_present(present),
              m_limit_16_19((limit & 0x00'0F'00'00) >> 16),
              m_avl(false),
              m_zero(false),
              m_32bit_segment(true),
              m_4kib_limit(limit_4kib),
              m_base_31_24((base & 0xFF'00'00'00) >> 24)
        {
        }
        gdtr::gdtr()
        {
        }
        gdtr::gdtr(capi::word limit, capi::adr base)
            : m_limit(limit),
              m_base(base)
        {
        }

        static gdtr s_gdtr;
        static utils::array<segment_descriptor, 3> s_descriptors;
        extern "C" void i686_load_gdt(gdtr);

        void init(capi::architecture* arch)
        {
            // Null descriptor
            s_descriptors[0] = segment_descriptor(0, 0, 0, false, false, false, 0);
            // Code segment descriptor
            s_descriptors[1] = segment_descriptor(
                0, 
                0xFF'FF'FF'FF, 
                0, 
                true, 
                true, 
                true, 
                FLAG(code_segment_flags::CODE)
            );
            // Data segment descriptor
            s_descriptors[2] = segment_descriptor(
                0,
                0xFF'FF'FF'FF,
                0, 
                true, 
                true, 
                true, 
                FLAG(data_segment_flags::DATA) | FLAG(data_segment_flags::WRITE)
            );
            /* TODO: Ugh, Code keeps complaining about this 
            reinterpret_cast being not of the right type,
            how to fucking tell it we are in i686 */
            s_gdtr = gdtr(
                sizeof(s_descriptors), 
                reinterpret_cast<capi::adr>(s_descriptors.data())
            );
            i686_load_gdt(s_gdtr);
            dbg_str(arch->get_io_interface(), "Loaded GDT\n");
        }
    }
}
