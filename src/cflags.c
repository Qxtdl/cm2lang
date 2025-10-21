#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "globals.h"
#include "cflags.h"

/*
 * TODO: Compiler flags
 *
 * [x] -fverbose-asm    Compiler comments
 * [x] -fstack-init     Value to initalize stack to
 * [ ] -fram-mask       RAM masks
*/

const char * const cflags[] = {
    "-fverbose-asm",
    "-fstack-init",
    "-fram-mask"
};

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