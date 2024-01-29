#ifndef INTERNAL_PS2_H
#define INTERNAL_PS2_H

#include <stddef.h>

extern char kbd_dn[128];
extern char kbd_dn_shifted[128];
extern char kbd_dn_alt[128];

typedef struct KBD_STATE {
  size_t alt;
  size_t shift;
  size_t caps;
  size_t ctrl;
} kbd_state;

extern kbd_state i_kbdstate;

#define KEY_CTL (0x1D)
#define KEY_ALT (0x38)
#define KEY_SHIFT (0x2A)
#define KEY_CAPS (0x3A)

#endif