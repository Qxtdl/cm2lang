#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../globals.h"
#include "../util.h"
#include "ir.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"

struct ir_state_s ir_state;

void ir_init(void)
{
    ir_state.compiled = malloc(1);
    *ir_state.compiled = '\0';
    ir_state.compiled_len = 0;
}

#define r_strcat(src) \
ir_state.compiled = realloc(ir_state.compiled, strlen(ir_state.compiled) + strlen(src) + 1); \
strcat(ir_state.compiled, src);

static void add_label(const char *name)
{
    r_strcat(name);
    r_strcat(":\r\n\t")
}

static void emit(
    const char *opcode, 
    const char *dest,
    const char *operand,
    const char *operand2,
    const char *comment
)
{
    r_strcat(opcode);
    r_strcat(" ");
    if (dest) {
        r_strcat(dest);
        r_strcat(", ");
    }
    if (operand) {
        r_strcat(operand);
        r_strcat(operand2 ? ", " : "");
    }
    if (operand2) {
        r_strcat(operand2);
    }
    if (comment) {
        r_strcat("\t# [COMPILER]: ");
        r_strcat(comment)
    }
    r_strcat("\r\n\t");
}

typedef struct {
    char *identifier;
    int address;
} var_entry_t;

size_t var_entry_stack_size = 0;
var_entry_t *var_entry_sp = NULL;

static var_entry_t *push_var(const char *identifier)
{
    var_entry_sp = realloc(var_entry_sp, sizeof(var_entry_t) * ++var_entry_stack_size);
    var_entry_sp[var_entry_stack_size - 1].identifier = identifier;
    var_entry_sp[var_entry_stack_size - 1].address = var_entry_stack_size == 1 ? 0 : var_entry_sp[var_entry_stack_size - 2].address + 1;
    return var_entry_sp;
}

static var_entry_t *lookup_var(char *identifier)
{
    for (size_t i = 0; i < var_entry_stack_size; i++)
        if (var_entry_sp[i].identifier == identifier)
            return &var_entry_sp[i];
    return NULL;
}

static int num_alloced_regs = 0;

static const char *alloc_reg(void)
{
    char *index = ir_registers[2 + num_alloced_regs++];
    if (2 + num_alloced_regs - 1 > sizeof(ir_registers) / sizeof(ir_registers[0]))
        ;//abort("alloc_reg()", "Register allocation overflow")
    return index;
}

static inline void free_reg(void)
{
    num_alloced_regs--;
}

static inline void n_free_reg(int num_regs_to_free)
{
    num_alloced_regs -= num_regs_to_free;
}

static bool ir_process_function(ast_node_t *fn_node)
{
    if (!fn_node) return false;
    add_label(fn_node->node_union.fn_node.name.ast_nodes[0]->node_union.name_node.value);
    if (!strcmp(fn_node->node_union.fn_node.name.ast_nodes[0]->node_union.name_node.value, "main"))
        emit("ldi", ir_registers[IR_SP], "65535", NULL, "Set up stack pointer");

    ast_node_t *node;
    while (node = ast_walk(&fn_node->node_union.fn_node.body)) {
        if (node->type == NODE_VARDECL) {
            const char *var_name = node->node_union.vardecl_node.name.ast_nodes[0]->node_union.name_node.value;
            push_var(var_name);

            ast_node_t *expr_op;
            for (int i = 0; expr_op = ast_walk(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.ops); i++) {
                ast_node_t *expr_operator = ast_walk(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.operator);
                char *reg_imm, *sp_subtrahend;
                if (i == 0) {
                    emit("ldi", reg_imm = alloc_reg(), expr_op->node_union.name_node.value, NULL, "Load immediate");
                    if (!strcmp(node->node_union.vardecl_node.type.ast_nodes[0]->node_union.name_node.value, "v16"))
                        emit("ldi", sp_subtrahend = alloc_reg(), "1", NULL, "Load subtrahend for stack pointer (v16)");
                    emit("sub", ir_registers[IR_SP], ir_registers[IR_SP], sp_subtrahend, "Manipulate stack pointer");
                    emit("sh", NULL, ir_registers[IR_SP], reg_imm, "Save value onto stack");
                    n_free_reg(2);
                }
                ast_node_t *next_expr_op = ast_peek(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.ops, 1);
                if (next_expr_op == NULL)
                    break;
                /* We have an operand to process */
                char *var_old_value;
                emit("ldi", reg_imm = alloc_reg(), next_expr_op->node_union.name_node.value, NULL, "Load second immediate");
                if (!strcmp(expr_operator->node_union.name_node.value, "+")) {
                    emit("load", var_old_value = alloc_reg(), ir_registers[IR_SP], NULL, "Load old var value");
                    emit("add", var_old_value, var_old_value, reg_imm, "Add old var value with imm");
                    emit("sh", NULL, ir_registers[IR_SP], var_old_value, "Save the result of addition");
                }
                n_free_reg(2);
            }

            // After var value is processed
        }
    }

    return true;
}

extern ast_node_t *program_node;

void ir_process(void)
{
    while (ir_process_function(ast_walk(&program_node->node_union.program_node.body)));
    ir_state.compiled_len = strlen(ir_state.compiled);
}
