#ifndef IR_H
#define IR_H

#include <stdlib.h>
#include <stdbool.h>

struct ir_state_s {
    char *compiled;
    const char *current_sp;
    size_t compiled_len;
};

#define IR_NOP 0
#define IR_LDI 1
#define IR_ADD 2
#define IR_SUB 3
#define IR_AND 4
#define IR_OR 5
#define IR_XOR 6
#define IR_SHR 7
#define IR_SHL 8
#define IR_LH 9
#define IR_SH 10
#define IR_BEQ 11

static const char * const ir_inst[] = {
    [IR_NOP]    =  "nop",
    [IR_LDI]    =  "ldi",
    [IR_ADD]    =  "add",
    [IR_SUB]    =  "sub",
    [IR_AND]    =  "and",
    [IR_OR]     =  "or",
    [IR_XOR]    =  "xor",
    [IR_SHR]    =  "shr",
    [IR_SHL]    =  "shl",
    [IR_LH]     =  "lh",
    [IR_SH]     =  "sh",
    [IR_BEQ]    =  "beq"
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
};

void ir_init(void);
void ir_process(void);

#endif // IR_H