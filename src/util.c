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

char *r_strcat(char *dest, const char *src)
{
   if (!dest || !src) return NULL;
   dest = realloc(dest, strlen(dest) + strlen(src) + 1);
   return strcat(dest, src);
}
