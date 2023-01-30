#include <string.h>
#include <error.h>
#include <stdio.h>
#include <sys.h>

void error_general(char const *details)
{
    fputs(details, stderr);
    stop();
}

void error_out_of_memory(char const *details)
{
    char const *prefix = "[Out of memory]";
    size_t preflen = strlen(prefix);
    size_t detlen = strlen(details);
    char buf[preflen + detlen + 2]; // +1 space +1 null termination
    if(!details)
    {
        error_general(prefix);
        return;
    }
    // Concatenating strings in C be like...
    strcpy(buf, prefix);
    strcat(buf, " ");
    strcat(buf, details);

    error_general(buf);
}
