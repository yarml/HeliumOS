#ifndef HELIUM_SYS_H
#define HELIUM_SYS_H

#include <asm/sys.h>

#define stop() as_sys_stop()

#define halt() asm("hlt")
#define pause() asm("pause")

#endif