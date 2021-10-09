#pragma once

#include <capi/types.hpp>
#include <multiboot.hpp>
#include <capi/architecture.hpp>
#include <capi/early_ordered_list.hpp>

namespace capi
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
        constexpr capi::adr limit() const
        {
            return m_limit;
        }
        constexpr multiboot::mem_type priv() const
        {
            return m_priv;
        }
        constexpr void set_priv(multiboot::mem_type new_priv)
        {
            m_priv = new_priv;
        }
    public:
        constexpr bool operator<(mem_boundary const& other) const
        {
            return m_limit < other.m_limit;
        }
        constexpr bool operator!=(mem_boundary const& other) const
        {
            return m_limit != other.m_limit || m_priv != other.m_priv;
        }
    private:
        capi::adr           m_limit;
        multiboot::mem_type m_priv ;
    };

    early_ordered_list<mem_boundary>& system_memory_map();
    void detect_memory(capi::architecture* arch, multiboot::info_structure* mbt_info);
    utils::ptr alloc_physical_memory(utils::size_type size);
    void free_physical_memory(utils::ptr ptr);
}
