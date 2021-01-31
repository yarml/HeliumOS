#include <dev/pic.hpp>
#include <dev/io.hpp>

namespace pic
{
    void send_eoi(uint8_t irq)
    {
        if(irq >= 8)
            outb(PIC_CMD(PIC_SLAVE), PIC_CMD_EOI);
        outb(PIC_CMD(PIC_MASTER), PIC_CMD_EOI);
    }
    void remap(uint32_t offset1, uint32_t offset2)
    {
        unsigned char a1, a2;
    
        a1 = inb(PIC_DATA(PIC_MASTER));
        a2 = inb(PIC_DATA(PIC_SLAVE));
    
        outb(PIC_CMD(PIC_MASTER), ICW1_INIT | ICW1_ICW4);
        io_wait();
        outb(PIC_CMD(PIC_SLAVE) , ICW1_INIT | ICW1_ICW4);
        io_wait();
        outb(PIC_DATA(PIC_MASTER), offset1);
        io_wait();
        outb(PIC_DATA(PIC_SLAVE) , offset2);
        io_wait();
        outb(PIC_DATA(PIC_MASTER), 4);
        io_wait();
        outb(PIC_DATA(PIC_SLAVE) , 2);
        io_wait();
    
        outb(PIC_DATA(PIC_MASTER), ICW4_8086);
        io_wait();
        outb(PIC_DATA(PIC_SLAVE) , ICW4_8086);
        io_wait();
    
        outb(PIC_DATA(PIC_MASTER), a1);
        outb(PIC_DATA(PIC_SLAVE) , a2);
    }
    void set_mask(uint8_t irq) 
    {
        uint16_t port;
        uint8_t value;
    
        if(irq < 8)
            port = PIC_DATA(PIC_MASTER);
        else 
        {
            port = PIC_DATA(PIC_SLAVE);
            irq -= 8;
        }
        value = inb(port) | (1 << irq);
        outb(port, value);        
    }  
    void clear_mask(uint8_t irq) 
    {
        uint16_t port;
        uint8_t value;
    
        if(irq < 8) {
            port = PIC_DATA(PIC_MASTER);
        } else {
            port = PIC_DATA(PIC_SLAVE);
            irq -= 8;
        }
        value = inb(port) & ~(1 << irq);
        outb(port, value);        
    }
}