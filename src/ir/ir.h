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

void ir_init();
void ir_process();

// Cm2 basic ir Reference
// =============================

// This ir consists of:
// - 1 stack
// - 20 instructions

// Instructions
// ------------

// 1. push_imm <immediate>
//    - Pushes an immediate value onto the stack.

// 2. push_ramaddr <ramaddr>
//    - Pushes the value stored at the given RAM address onto the stack.

// 3. peek
//    - Reads the value on top of the stack and pushes the RAM address that the value points to.

// 4. poke
//    - Pops two values from the stack: 
//      1: RAM address
//      2: New value
//    - Sets the memory at the RAM address to the new value.

// 5. eq
//    - Pops two values from the stack, pushes 1 if equal, 0 if not.

// 6. neq
//    - Pops two values, pushes 1 if not equal, 0 if equal.

// 7. gt
//    - Pops two values, pushes 1 if the first > second, 0 otherwise.

// 8. gte
//    - Pops two values, pushes 1 if first >= second, 0 otherwise.

// 9. lt
//    - Pops two values, pushes 1 if first < second, 0 otherwise.

// 10. lte
//     - Pops two values, pushes 1 if first <= second, 0 otherwise.

// 11. add
//     - Pops two values, pushes the sum.

// 12. sub
//     - Pops two values, pushes the difference (first - second).

// 13. mul
//     - Pops two values, pushes the product.

// 14. idiv
//     - Pops two values, pushes the integer division result (first / second).

// 15. jz <label>
//     - Pops a value; jumps to the label if the value is 0.

// 16. jnz <label>

//     - Pops a value; jumps to the label if the value is not 0.
// 17. jmp <label>

//     - jumps to label
// 18. call <label>

//     - calls an label and stores it in call stack not main stack
// 19. ret

//     - pops from calls stack and jumps 
// 20. asm "asmcode"

//     - directly insert asmcode 

// Notes
// -----
// - Every operation uses the stack except call and ret they use call stack.
// - Arithmetic and comparison operations always pop the required number of values and push the result back.

////////////////////////////////////////////////////////////////

#endif // IR_H