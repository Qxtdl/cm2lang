#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../globals.h"
#include "ir.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"

struct ir_state_s ir_state;

#define r_strcat(src) \
ir_state.compiled = realloc(ir_state.compiled, strlen(ir_state.compiled) + strlen(src) + 1); \
strcat(ir_state.compiled, src);

void ir_init()
{
    ir_state.compiled = malloc(1);
    *ir_state.compiled = '\0';
    ir_state.compiled_len = 0;
}

static void add_label(const char *name)
{
    r_strcat(name);
    r_strcat(":\r\n\t")
}

static void emit(
    const char *opcode, 
    const char *dest,
    const char *operand,
    const char *operand2
)
{
    r_strcat(opcode);
    if (dest)
    {
        r_strcat(dest);
        r_strcat(" ");
    }
    if (operand)
    {
        r_strcat(operand);
        r_strcat(" ");
    }
    if (operand2)
    {
        r_strcat(operand2);
        r_strcat(" ");
    }
    r_strcat("\r\n\t");
}

extern ast_node_t *program_node;
extern ast_node_t *last_created_fn_node;

static bool ir_process_function(ast_node_t *fn_node)
{
    if (!fn_node) return false;

    add_label(fn_node->node_union.fn_node.name.ast_nodes[0]->node_union.name_node.value);

    ast_node_t *node;
    while (node = ast_walk(&fn_node->node_union.fn_node.body))
    {
        if (node->type == NODE_VARDECL)
        {
            bool next_operator_is_plus = false;
            size_t evaluated_imm = 0;
            ast_node_t *ops; 
            while (ops = ast_walk(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.ops))
            {
                ast_node_t *expr_operator = ast_walk(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.operator);
                const char *operator = NULL;
                if (expr_operator)
                    operator = expr_operator->node_union.name_node.value;
                const char *op = ops->node_union.name_node.value;
                debug_printf("op: %s\n", op);
                if (isdigit(*op))
                {
                    if (operator == NULL && !next_operator_is_plus)
                    {
                        debug_printf("Operator is null\n");
                        char *endptr;
                        evaluated_imm = strtoul(op, &endptr, 10);
                        continue;
                    }
                    if (next_operator_is_plus || !strcmp(operator, "+"))
                    {
                        if (next_operator_is_plus)
                            debug_printf("next operator is +");
                        debug_printf("Operator is +\n");
                        char *endptr;
                        evaluated_imm += strtoul(op, &endptr, 10);
                        if (!next_operator_is_plus)
                            next_operator_is_plus = true;
                        if (next_operator_is_plus)
                            next_operator_is_plus != 1;       
                        continue;
                    }
                }
            }
            debug_printf("Evaluated immediate: %d\n", evaluated_imm);
        }
    }

    return true;
}

void ir_process()
{
    while (ir_process_function(ast_walk(&program_node->node_union.program_node.body)));
    ir_state.compiled_len = strlen(ir_state.compiled);
}
