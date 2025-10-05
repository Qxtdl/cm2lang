#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../globals.h"
#include "ir.h"
#include "../lexer/lexer.h"

struct ir_state_s ir_state;

const char *opcodes[] = {
    "push_8",   // IR_INST_PUSH_8
    "push_16",  // IR_INST_PUSH_16
    "push_32",  // IR_INST_PUSH_32

    "pop_8",    // IR_INST_POP_8
    "pop_16",   // IR_INST_POP_16
    "pop_32",   // IR_INST_POP_32
};

#define r_strcat(dest, src) \
dest = realloc(dest, strlen(dest) + strlen(src) + 1); \
Check_alloc_fail(dest, exit(1)) \
strcat(dest, src);

void ir_init(void)
{
    ir_state.compiled = malloc(1);
    Check_alloc_fail(ir_state.compiled, exit(1))
    *ir_state.compiled = '\0';
    ir_state.compiled_len = 0;
}

static void add_label(const char *name)
{
    r_strcat(ir_state.compiled, name)
    r_strcat(ir_state.compiled, ":\r\n\t")
}

static void add_asm(ir_inst_t inst)
{
    r_strcat(ir_state.compiled, opcodes[inst.opcode])
    if (inst.use_imm) 
    {
        r_strcat(ir_state.compiled, " ")
        r_strcat(ir_state.compiled, inst.imm)
    }
    r_strcat(ir_state.compiled, "\r\n\t")
}

void ir_process(void)
{
    bool ir_continue = true;
    while (ir_continue)
    {
        token_t token = lexer_read_token(&ir_continue);
        if (!ir_continue) break; // TOKEN_EOF
        switch (token.type)
        {
            case TOKEN_MAIN_FUNCTION:
                add_label("main"); break;
            case TOKEN_U8:
                lexer_advance_token();
                lexer_advance_token();
                add_asm((ir_inst_t){IR_INST_PUSH_8, 0, 0, 0, lexer_read_token(&ir_continue).value, 1});
        }
        
        free(token.value);
    }
    ir_state.compiled_len = strlen(ir_state.compiled);
    debug_printf("////\n%s\n////\n", ir_state.compiled);
}
