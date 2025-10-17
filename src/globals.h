#ifndef GLOBALS_H
#define GLOBALS_H

#define debug_printf(...) printf(__VA_ARGS__)

#define abort(func_name, reasson) \
fprintf(stderr, "[ABORT]: %s:" func_name ":%d: " reasson "\n", __FILE_NAME__, __LINE__); \
exit(1);

#endif // GLOBALS_H