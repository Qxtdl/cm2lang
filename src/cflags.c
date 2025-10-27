#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "globals.h"
#include "cflags.h"

/*
 * TODO: Compiler flags
 *
 * [x] -fverbose-asm        Compiler comments
 * [x] -fstack-init         Value to initalize stack to
 * [ ] -fram-mask           RAM masks
 * [x] -fasm-comment        Comment for assembly
 * [ ] -fhalf-size          How big a 16 bit value is in terms of address space
 * [x] -fno-init-zero-reg   Initalize zero reg to zero incase it's not hardwired to be zero
*/

bool is_cflag_enabled(const char *cflag)
{
    size_t cflag_length = strlen(cflag);
    for (int i = 2; i < g_argc; i++) 
        if (!strncmp(g_argv[i], cflag, cflag_length))
            return true;
    return false;
}

const char *get_cflag_value(const char *cflag)
{
    size_t cflag_length = strlen(cflag);
    for (int i = 2; i < g_argc; i++) 
        if (!strncmp(g_argv[i], cflag, cflag_length)) 
        {
            char *value = g_argv[i];
            while (*value != '=') value++;
            return ++value;
        }
    return NULL;
}