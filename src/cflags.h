#ifndef CFLAGS_H
#define CFLAGS_H

#include <stdbool.h>

bool is_cflag_enabled(const char *cflag);
const char *get_cflag_value(const char *cflag);

#endif // CFLAGS_H