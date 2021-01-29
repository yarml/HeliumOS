#ifndef OUT_HPP
#define OUT_HPP

#include <stdint.h>

extern "C" void     outb(uint16_t port, uint8_t  data);
extern "C" void     outw(uint16_t port, uint16_t data);
extern "C" uint8_t  inb( uint16_t port               );
extern "C" uint16_t inw( uint16_t port               );


#endif