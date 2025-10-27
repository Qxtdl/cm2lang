#include <string.h>

#include "globals.h"
#include "util.h"

int s_strcmp(const char *first, const char *second)
{
    if (!first || !second) { app_abort("s_strcmp()", "first or second are nullptrs") }
    return strcmp(first, second);
}

const char *itoa(int n) 
{
    static char buf[128];
    snprintf(buf, sizeof(buf), "%d", n);
    return buf;
}
