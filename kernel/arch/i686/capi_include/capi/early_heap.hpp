#pragma once

#include <utils/types.hpp>
#include <utils/array.hpp>
#include <utils/mem.hpp>
#include <config/memory.hpp>
#include <utils/maths.hpp>

namespace capi
{
    // allocation_unit and heap_size in bytes
    template <size_t allocation_unit, size_t heap_size>
    class early_heap
    {
    public:
        using                           data_type   = capi::byte     ;
        using                           bitmap_type = capi::byte     ;
        using                           unit_type   = size_t         ;
        constexpr static unit_type      unit        = allocation_unit;
        constexpr static capi::mem_size size        = heap_size      ;
    private:
        constexpr static unit_type INVALID_UNIT = 0xFF'FF'FF'FF;
        constexpr static size_t heap_units = size / unit;
        constexpr static size_t bitmap_len = size / unit / utils::type_meta<data_type>::size();
        constexpr static utils::bit_index bit(unit_type u)
        {
            return u % utils::type_meta<bitmap_type>::size();
        }
        constexpr static capi::mem_size byte(unit_type u)
        {
            return u / utils::type_meta<bitmap_type>::size();
        }
        constexpr static unit_type units_count(capi::mem_size byte_len)
        {
            return byte_len / unit + (byte_len % unit == 0 ? 0 : 1);
        }
        constexpr utils::ptr unit_to_ptr(unit_type u)
        {
            return u * unit + m_heap.data();
        }
        constexpr unit_type ptr_to_unit(utils::ptr p)
        {
            return 
                (
                    reinterpret_cast<data_type*>(p) 
                    - reinterpret_cast<data_type*>(m_heap.data())
                ) 
                / unit;
        }
    public:
        early_heap() 
        {
            ::mem::set(m_bitmap.data(), 0, m_bitmap.size());
        }
    private:
        constexpr bool available(unit_type unit)
        {
            return !((m_bitmap[byte(unit)] >> bit(unit)) & 1);
        }
        constexpr void mark(unit_type unit)
        {
            m_bitmap[byte(unit)] |= 1 << bit(unit);
        }
        constexpr void unmark(unit_type unit)
        {
            m_bitmap[byte(unit)] &= ~(1 << bit(unit));
        }
    public:
        utils::ptr alloc(unit_type len)
        {
            unit_type first_unit = INVALID_UNIT;
            unit_type available_count = 0;
            for(unit_type i = 0; i < heap_units; ++i)
            {
                if(available(i))
                {
                    ++available_count;
                    if(first_unit == INVALID_UNIT)
                        first_unit = i;
                    if(available_count == units_count(len))
                    {
                        for(unit_type j = first_unit; j < first_unit + units_count(len); ++j)
                            mark(j);
                        return unit_to_ptr(first_unit);
                    }
                }
                else
                {
                    available_count = 0;
                    first_unit = INVALID_UNIT;
                }
            }
            return nullptr;
        }
        void free(utils::ptr p, unit_type len)
        {
            for(unit_type i = ptr_to_unit(p); i < ptr_to_unit(p) + units_count(len); ++i)
                unmark(i);
        }
        /// This implementation is straight forward
        /// Allocate new buffer -> copy data -> free old buffer -> return
        /// This implementation is faster, but can cause fragmentation
        /// Use realloc2(...) if you don't want fragmentation at the expanse of speed
        utils::ptr realloc(utils::ptr p, unit_type old_len, unit_type new_len)
        {
            auto copylen = maths::min(old_len, new_len);
            utils::ptr new_p = alloc(new_len);
            ::mem::copy(new_p, p, copylen);
            free(p, old_len);
            return new_p;
        }
        /// This implementation is slow, but helps preventing fragmentation
        /// Copy data to an internal buffer -> free old buffer -> allocate new buffer
        /// -> copy data to new buffer -> return
        /// use realloc(...) if you want to optimise for speed at the expanse of potentially
        /// fragmenting memory
        utils::ptr realloc2(utils::ptr p, unit_type old_len, unit_type new_len)
        {
            auto copylen = maths::min(old_len, new_len);
            data_type buf[copylen];
            ::mem::copy(buf, p, copylen);
            free(p);
            auto new_p = alloc(new_len);
            ::mem::copy(new_p, buf, copylen);
            return new_p;
        }
    private:
        utils::array<data_type , heap_units > m_heap  ;
        utils::array<bitmap_type, bitmap_len> m_bitmap;
    };
    using namespace i686::mem::early_heap_config;
    using std_early_heap = early_heap<UNIT, SIZE>;

    std_early_heap* system_early_heap();
}