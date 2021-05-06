#include <memory/memory.hpp>


namespace memory
{
    page_table_entry::page_table_entry()
    {
        _flags = 0;
        _adr_low_free = 0;
        _adr_high = 0;
    }
    page_table_entry::page_table_entry(uint32_t phy_page, uint8_t flags)
    {
        _flags = flags;
        _adr_high = (phy_page * 4096 & 0xFFFFF000) >> 16;
        _adr_low_free = (phy_page * 4096 & 0xFFFFF000) >> 8;
    }
    uint32_t page_table_entry::physical_page()
    {
        return (_adr_low_free >> 4) | (_adr_high << 4);
    }
    void page_table_entry::set_flags(uint8_t flags)
    {
        _flags = flags;
    }
    page_directory_entry::page_directory_entry()
    {
        _flags = 0;
        _adr_low_free = 0;
        _adr_high = 0;
    }
    page_directory_entry::page_directory_entry(page_table_entry *first, uint8_t flags)
    {
        _flags = flags;
        _adr_high = ((uint32_t)first & 0xFFFFF000) >> 16;
        _adr_low_free = ((uint32_t)first & 0xFFFFF000) >> 8;
    }
    page_directory_entry::operator uint32_t()
    {
        return (_adr_high << 16) | (_adr_low_free << 8) | _flags;
    }
    void page_directory_entry::set_flags(uint8_t flags)
    {
        _flags = flags;
    }
    page_table_entry &page_directory_entry::at(uint32_t framen)
    {
        page_table_entry *pt = (page_table_entry *)(((uint32_t)_adr_high << 16 | (uint32_t)_adr_low_free << 8) + 0xC0000000);
        return *(pt + framen);
    }
    page_table_entry &page_directory_entry::operator[](uint32_t framen)
    {
        return at(framen);
    }
    bool page_directory_entry::get_flag(page_struct_flags flag)
    {
        return _flags & flag;
    }
    bool page_directory_entry::present()
    {
        return get_flag(page_struct_flags::PRESENT);
    }
    void page_directory_entry::set_flag(page_struct_flags flag)
    {
        _flags |= flag;
    }
    bool page_table_entry::get_flag(page_struct_flags flag)
    {
        return _flags & flag;
    }
    bool page_table_entry::present()
    {
        return get_flag(page_struct_flags::PRESENT);
    }
    void page_table_entry::set_flag(page_struct_flags flag)
    {
        _flags |= flag;
    }
}