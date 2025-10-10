#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../globals.h"
#include "ir.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"

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
strcat(dest, src);

void ir_init(void)
{
    ir_state.compiled = malloc(1);
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

extern ast_node_t *program_node;
extern ast_node_t *last_created_fn_node;

static bool ir_process_function(ast_node_t *fn_node)
{
    if (!fn_node) return false;

    add_label(fn_node->node_union.fn_node.name.ast_nodes[0]->node_union.name_node.value);

    ast_node_t *node;
    while ((node = ast_walk(&fn_node->node_union.fn_node.body)))
    {
        //printf("%d\n", node->type)
    }

    return true;
}

void ir_process(void)
{
    while (ir_process_function(ast_walk(&program_node->node_union.program_node.body)));
    ir_state.compiled_len = strlen(ir_state.compiled);
}
