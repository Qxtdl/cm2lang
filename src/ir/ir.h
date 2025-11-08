#ifndef IR_H
#define IR_H

#include <stdlib.h>
#include <stdbool.h>

struct ir_state {
    char *compiled;
    size_t compiled_len;
};

enum {
    IR_NOP,
    IR_LDI,
    IR_ADD,
    IR_SUB,
    IR_AND,
    IR_OR,
    IR_XOR,
    IR_SHR,
    IR_SHL,
    IR_LH,
    IR_SH,
    IR_JMP,
    IR_BEQ,
    IR_CALL,
    IR_RET
};

static const char * const ir_inst[] = {
    [IR_NOP]    = "nop",
    [IR_LDI]    = "ldi",
    [IR_ADD]    = "add",
    [IR_SUB]    = "sub",
    [IR_AND]    = "and",
    [IR_OR]     = "or",
    [IR_XOR]    = "xor",
    [IR_SHR]    = "shr",
    [IR_SHL]    = "shl",
    [IR_LH]     = "lh",
    [IR_SH]     = "sh",
    [IR_JMP]    = "jmp",
    [IR_BEQ]    = "beq",
    [IR_CALL]   = "call",
    [IR_RET]    = "ret"
};

#define IR_ZERO 0
#define IR_SP 1
#define IR_T0 2
#define IR_T1 3
#define IR_T2 4
#define IR_T3 5
#define IR_T4 6

typedef struct {
    const char *const name;
    bool busy;
} ir_register_t;

void ir_init(void);
void ir_process(void);

#endif // IR_H