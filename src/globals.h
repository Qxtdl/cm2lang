#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdlib.h>
#include <stdio.h>

#define debug_printf(...) printf(__VA_ARGS__)

#define app_abort(func_name, reasson, ...) \
fprintf(stderr, "[ABORT]: %s:" func_name ":%d: " reasson "\n", __FILE_NAME__, __LINE__, ##__VA_ARGS__); \
exit(1);

#define quick_abort(reasson, ...) \
fprintf(stderr, "[ABORT]: %s:%d: " reasson "\n", __FILE_NAME__, __LINE__, ##__VA_ARGS__); \
exit(1);

#define NOT_IMPLEMENTED quick_abort("NOT IMPLEMENTED!")

extern int g_argc;
extern char **g_argv;

#endif // GLOBALS_H