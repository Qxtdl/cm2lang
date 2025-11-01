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

extern void compiler_warn(const char *message);

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
    if (!strcmp(cflag, cflags[FLAG_STACK_INIT])) {
        compiler_warn("This message may be printed repeatedly! You did not provide -fstack-init flag. Defaulting to 0");
        return "0";
    }
    if (!strcmp(cflag, cflags[FLAG_HALF_SIZE])) {
        compiler_warn("This message may be printed repeatedly! You did not provide -fhalf-size flag. Defaulting to 2 (works on byte addressed systems)");        
        return "2";
    }
    app_abort("get_cflag_value()", "Could not provide a flag value for %s", cflag)
}