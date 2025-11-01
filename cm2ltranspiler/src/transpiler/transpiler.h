#ifndef TRANSPILER_H
#define TRANSPILER_H

#include <stdlib.h>

struct transpiler_state {
    char *transpiled;
    size_t transpiled_len;
    char *asm_source;
};

void transpiler_init(char *asm_source);
void transpiler_process(void);

#endif // TRANSPILER_H