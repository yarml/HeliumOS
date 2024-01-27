#ifndef HELIUM_DEV_PS2_H
#define HELIUM_DEV_PS2_H

#define PS2_KBD_INTVEC (0xD0)

void ps2_init();

#define PS2_PORT_DATA (0x60)
#define PS2_PORT_STAT (0x64)
#define PS2_PORT_CMND (0x64)

#endif