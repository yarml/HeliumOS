#pragma once

#include <config/memory.hpp>
#include <support/attributes.hpp>
#include <capi/types.hpp>
#include <capi/architecture.hpp>
#include <multiboot.hpp>
#include <utils/array.hpp>
#include <utils/mem.hpp>
#include <utils/maths.hpp>

namespace i686::mem
{
    namespace gdt
    {
        enum class data_segment_flags : capi::byte
        {
            DATA  = 0b0000,
            EXPD  = 0b0100,
            WRITE = 0b0010,
            ACCES = 0b0001
        };
        enum class code_segment_flags : capi::byte
        {
            CODE  = 0b1000,
            CONF  = 0b0100,
            READ  = 0b0010,
            ACCES = 0b0001
        };
        typedef capi::byte segment_type;
        constexpr segment_type FLAG(data_segment_flags f)
        {
            return static_cast<segment_type>(f);
        }
        constexpr segment_type FLAG(code_segment_flags f)
        {
            return static_cast<segment_type>(f);
        }
        class segment_descriptor
        {
        public:
            segment_descriptor(); // To make the compiler stfu
            segment_descriptor(
                capi::adr base,
                capi::mem_size limit, 
                capi::byte dpl, 
                bool present, 
                bool limit_4kib, 
                bool code_data, 
                segment_type type
            );
        private:
            // TODO: gcc produces a lot of warnings here about the UNUSED attribute
            capi::word m_limit_15_00    UNUSED  ;
            capi::word m_base_15_00     UNUSED  ;
            capi::byte m_base_23_16     UNUSED  ;
            capi::byte m_segment_type   UNUSED:4;
            bool       m_code_data      UNUSED:1;
            capi::byte m_dpl            UNUSED:2;
            bool       m_present        UNUSED:1;
            capi::byte m_limit_16_19    UNUSED:4;
            bool       m_avl            UNUSED:1;
            bool       m_zero           UNUSED:1;
            bool       m_32bit_segment  UNUSED:1;
            bool       m_4kib_limit     UNUSED:1;
            capi::byte m_base_31_24     UNUSED  ;
        } PACKED; // ugh, Why does Code think this is a variable name and not a macro
        class gdtr
        {
        public:
            gdtr();
            gdtr(capi::word limit, capi::adr base);
        private:
            capi::word m_limit UNUSED;
            capi::adr  m_base  UNUSED;
        } PACKED;
        void init(capi::architecture* arch);
    }

    void init(capi::architecture* arch, multiboot::info_structure* mbt_info);
}
