#include <stdint.h>

enum gdt_flags
{
    GRANULARITY = 0b10000000,
    OP_SIZE     = 0b01000000
};

enum gdt_access
{
    PRESENT   = 0b10000000,
    DPL_1     = 0b00100000,
    DPL_2     = 0b01000000,
    DPL_3     = 0b01100000,
    CODE_DATA = 0b00010000,
    EXEC      = 0b00001000,
    DIRECTION = 0b00000100,
    RW        = 0b00000010,
    ACCESS    = 0b00000001
};

class gdt_entry
{
private:
    uint16_t m_limit_low  ;
    uint16_t m_base_low   ;
    uint8_t  m_base_middle;
    uint8_t  m_access     ;
    uint8_t  m_granularity;
    uint8_t  m_base_high  ;
public:
    gdt_entry() { }
    gdt_entry(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
    {
        m_limit_low    = ((limit   & 0x0000FFFF)      );
        m_granularity  = ((limit   & 0x000F0000) >> 16);
        m_base_middle  = ((base    & 0x00FF0000) >> 16);
        m_base_low     = ((base    & 0x0000FFFF)      );
        m_base_high    = ((base    & 0xFF000000) >> 24);
        m_access       = (access                      );
        m_granularity |= (flags                       );
    }
} __attribute__((packed));

struct gdt 
{
    uint16_t   size  ;
    gdt_entry* offset;
} __attribute__((packed));

static gdt_entry entries[3];

static gdt gdtr;

extern "C" void load_gdt(gdt);

extern "C" void setup_gdt()
{
    entries[0] = gdt_entry(0, 0, 0, 0);
    entries[1] = gdt_entry(0x00000000, 0xFFFFFFFF, 
                                   gdt_access::PRESENT    | gdt_access::CODE_DATA | 
                                   gdt_access::EXEC       | gdt_access::RW        ,
                                   gdt_flags::GRANULARITY | gdt_flags::OP_SIZE    );
    entries[2] = gdt_entry(0x00000000, 0xFFFFFFFF,
                                   gdt_access::PRESENT                    | gdt_access::CODE_DATA|
                                   gdt_access::RW, gdt_flags::GRANULARITY | gdt_flags::OP_SIZE   );
    gdtr = { .size = sizeof(entries), .offset = entries };

    load_gdt(gdtr);

}