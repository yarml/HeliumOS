#ifndef HELIUM_ERROR_H
#define HELIUM_ERROR_H

#include <attributes.h>

typedef int   errno_t;

noreturn void error_general(char *prefix, char *details);
noreturn void error_out_of_memory(char *details);
noreturn void error_inv_state(char *details);

#endif