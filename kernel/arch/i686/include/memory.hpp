#pragma once

#include <support/attributes.hpp>
#include <capi/types.hpp>


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
            UNUSED capi::word m_limit_15_00      ;
            UNUSED capi::word m_base_15_00       ;
            UNUSED capi::byte m_base_23_16       ;
            UNUSED capi::byte m_segment_type   :4;
            UNUSED bool       m_code_data      :1;
            UNUSED capi::byte m_dpl            :2;
            UNUSED bool       m_present        :1;
            UNUSED capi::byte m_limit_16_19    :4;
            UNUSED bool       m_avl            :1;
            UNUSED bool       m_zero           :1;
            UNUSED bool       m_32bit_segment  :1;
            UNUSED bool       m_4kib_limit     :1;
            UNUSED capi::byte m_base_31_24       ;
        } PACKED;
        class gdtr
        {
        public:
            gdtr();
            gdtr(capi::word limit, capi::adr base);
        private:
            UNUSED capi::word m_limit;
            UNUSED capi::adr  m_base ;
        } PACKED;
        void init();
    }
}
