#ifndef UTIL_H
#define UTIL_H

// #define r_strcat(dest, src) \
// if (dest && src) { \
//    (dest) = realloc((dest), strlen((dest)) + strlen((src)) + 1); \
//    strcat((dest), (src)); }

char *r_strcat(dest, src);
char *strreplace(char *string, const char *target, const char *replacement);

#endif