#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <stdint.h>
#include <multiboot.h>
#include <kmath.hpp>

#define MEMORY_MAX_SIZE (0xFFFFFFFF)
#define PAGES_MAX_COUNT (MEMORY_MAX_SIZE / 4096)
#define INVALID_PAGE ((uint32_t) -1)

#define PAGE_DIRS_COUNT (1024)
#define PAGE_TABLES_PER_DIR (1024)

namespace memory
{

    extern uint32_t memory_size;
    enum page_struct_flags : uint8_t
    {
        PRESENT = 0b00000001,
        RW      = 0b00000010,
        USERM   = 0b00000100,
        PWT     = 0b00001000,
        PCD     = 0b00010000,
        PAT     = 0b10000000
    };
    // I'll only implement 4Kib pages since I don't need 4Mib pages
    // No global pages, no PAE
    struct page_table_entry
    {
    private:
        uint8_t  _flags       ;
        uint8_t  _adr_low_free;
        uint16_t _adr_high    ;
    public:
        page_table_entry();
        page_table_entry(uint32_t page, uint8_t flags);
    public:
        void set_flags(uint8_t flags);
        void set_flag(page_struct_flags flag);
    public:
        uint32_t physical_page();
    public:
        bool get_flag(page_struct_flags flag);
        bool present();
    }__attribute__((packed));

    struct page_directory_entry
    {
    public:
        uint8_t  _flags       ;
        uint8_t  _adr_low_free;
        uint16_t _adr_high    ;
    public:
        page_directory_entry();
        page_directory_entry(page_table_entry* first, uint8_t flags);
    public:
        void set_flags(uint8_t flags);
        void set_flag(page_struct_flags flag);
    public:
        bool get_flag(page_struct_flags flag);
        bool present();
    public:
        page_table_entry& at(uint32_t framen);
    public:
        page_table_entry& operator[](uint32_t framen);
        operator uint32_t();
    }__attribute__((packed));
    
    bool init(multiboot_info_t* mbt);
    void mark_page(  uint32_t page);
    void unmark_page(uint32_t page);
    uint32_t mark_next_page();
    uint32_t physical_page(uint32_t page);
    uint32_t kallocvp(uint32_t count = 1);
    void freevp(uint32_t start, uint32_t count = 1);
}

#endif