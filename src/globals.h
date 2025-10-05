#ifndef GLOBALS_H
#define GLOBALS_H

#define debug_printf(...) printf(__VA_ARGS__)

#define Check_alloc_fail(var, action) \
if (!var) { \
    perror("Failed to allocate memory"); \
    action; \
}

#endif // GLOBALS_H