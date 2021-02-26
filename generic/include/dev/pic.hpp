#ifndef PIC_HPP
#define PIC_HPP

#include <stdint.h>

#define PIC_MASTER   0x0020
#define PIC_SLAVE    0x00A0

#define PIC_CMD(PIC)  (PIC    )
#define PIC_DATA(PIC) (PIC + 1)

#define PIC_CMD_EOI 0x20
 
#define ICW1_ICW4	    0x01
#define ICW1_SINGLE	    0x02
#define ICW1_INTERVAL4	0x04
#define ICW1_LEVEL	    0x08
#define ICW1_INIT	    0x10
 
#define ICW4_8086	    0x01
#define ICW4_AUTO	    0x02
#define ICW4_BUF_SLAVE	0x08
#define ICW4_BUF_MASTER	0x0C
#define ICW4_SFNM	    0x10

#define PIC_MASTER_OFFSET 32
#define PIC_SLAVE_OFFSET  (PIC_MASTER_OFFSET + 8)

enum irq : uint8_t
{
    PIT,
    KEYBOARD,
    PIC2,
    COM2,
    COM1,
    LPT2,
    FLOPPY,
    LPT1,
    RT_CLOCK,
    FREE1,
    FREE2,
    FREE3,
    PS_MOUSE,
    FPU,
    PRIM_ATA_HARD_DISK,
    SEC_ATA_HARD_DISK
};

namespace pic
{
    void remap(     uint32_t offset1, uint32_t offset2);
    void set_mask(  uint8_t irq                       );
    void clear_mask(uint8_t irq                       );
    void send_eoi(  uint8_t irq                       );

}

#endif /* PIC_HPP */