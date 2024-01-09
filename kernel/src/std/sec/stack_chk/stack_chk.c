#include <attributes.h>
#include <mutex.h>
#include <stdint.h>
#include <stdio.h>
#include <sys.h>

uintptr_t __stack_chk_guard = 0x595e9fbd94fda766;

// __stack_chk_fail runs on its own stack
// If a stack corruption occurs, we can't assume the stack pointer is
// not far from reaching its limit, therefor, we give __stack_chk_fail a little
// bit of its own space.
uint8_t __stack_chk_stack[1024];

// Now there would be a problem if 2 cores detect a stack smash at the same
// time... they would both use the same stack for __stack_chk_fail, this is a
// big no no When an stack smashing happens, the worst thing that can follow is
// no error message being displayed If two cores stack smash at the same time,
// and then both try to use the same stack while generating the error message,
// we may not see any error message. Therefore, only the first of them that can
// lock this mutex will be able to use the stack. We won't see that 2 or more
// cores stack smashed, but at least we would see that a stack smahing happened
mutex __stack_chk_lock;

// This is called from assembly
void __stack_chk_fail_p2() { printd("[Proc %&] Stack smashing detected"); }
