#ifndef HELIUM_ASM_IO_H
#define HELIUM_ASM_IO_H

#include <stddef.h>
#include <stdint.h>


void as_outb(uint16_t port, uint8_t data);
void as_outsb(uint16_t port, uint8_t *data, size_t len);

#endif
