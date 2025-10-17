#include "globals.h"
#include "util.h"

int s_strcmp(const char *first, const char *second)
{
    if (!first || !second) return;
    return strcmp(first, second);
}

const char *itoa(int n) 
{
    static char buf[128];
    snprintf(buf, sizeof(buf), "%d", n);
    return buf;
}
