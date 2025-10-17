#ifndef IR_H
#define IR_H

#include <stdlib.h>
#include <stdbool.h>

struct ir_state_s {
    char *compiled;
    size_t compiled_len;
};

#define IR_ZERO 0
#define IR_SP 1
#define IR_T0 2
#define IR_T1 3
#define IR_T2 4

static const char * const ir_registers[] = {
    [0] = "x0", // zero
    [1] = "x1", // sp (points to half)
    [2] = "x2", // t0
    [3] = "x3", // t1
    [4] = "x4", // t2
    [5] = "x5", // t3
    [6] = "x6", // t4
};

void ir_init(void);
void ir_process(void);

#endif // IR_H