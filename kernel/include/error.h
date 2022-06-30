#ifndef HELIUM_ERROR_H
#define HELIUM_ERROR_H

typedef int errno_t;


void error_general(char const* details);
void error_out_of_memory(char const* details);

#endif