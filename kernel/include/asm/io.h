#ifndef HELIUM_ASM_IO_H
#define HELIUM_ASM_IO_H

#include <stddef.h>
#include <stdint.h>

void as_outb(uint16_t port, uint8_t data);
void as_outsb(uint16_t port, uint8_t *data, size_t len);

void as_outw(uint16_t port, uint16_t data);
void as_outsw(uint16_t port, uint16_t *data, size_t len);

void as_outd(uint16_t port, uint32_t data);
void as_outsd(uint16_t port, uint32_t *data, size_t len);

uint8_t as_inb(uint16_t port);
void    as_insb(uint16_t port, uint8_t *data, size_t len);

uint16_t as_inw(uint16_t port);
void     as_insw(uint16_t port, uint16_t *data, size_t len);

uint32_t as_ind(uint16_t port);
void     as_insd(uint16_t port, uint32_t *data, size_t len);

#endif
