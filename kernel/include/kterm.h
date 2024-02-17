#ifndef HELIUM_KTERM_H
#define HELIUM_KTERM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void   kterm_init();
size_t kterm_print(char const *str);
size_t kterm_read(char *restrict str, size_t n);

// Put character/string in the input buffer
void kterm_putcin(char c);
void kterm_putsin(char const *str);

void kterm_clear();

void kterm_setfg(uint8_t r, uint8_t g, uint8_t b);
void kterm_setbg(uint8_t r, uint8_t g, uint8_t b);

void kterm_flush();
void kterm_dfb(bool usedfb);

#endif