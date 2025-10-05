#ifndef IR_H
#define IR_H

#include <stdlib.h>
#include <stdbool.h>

struct ir_state_s {
    char *compiled;
    size_t compiled_len;
};

typedef enum {
    IR_INST_PUSH_8,
    IR_INST_PUSH_16,
    IR_INST_PUSH_32,

    IR_INST_POP_8,
    IR_INST_POP_16,
    IR_INST_POP_32,
} ir_inst_opcode_t;

typedef struct {
    ir_inst_opcode_t opcode;
    int dest, op1, op2;
    const char *imm;
    bool use_imm;
} ir_inst_t;

void ir_init(void);
void ir_process(void);

/*
 * lets say our target is a simple machine with this isa
 *  
 * registers
 * x1 - x4
 * 
 * opcode
 * li dest, imm
 * add dest, op1, op2
 * sub dest, op1, op2
 * load dest, op1(addr)
 * store op1(addr), op2(value)
 * push op1
 * pop dest
*/

#endif // IR_H