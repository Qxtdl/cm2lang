#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../globals.h"
#include "../util.h"
#include "ir.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../cflags.h"

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
    r_strcat(":\r")
}

static void emit(
    const char *opcode, 
    const char *dest,
    const char *operand,
    const char *operand2,
    const char *comment
    // MIGHT DO: variable args for better comments
)
{
    r_strcat("\t")
    r_strcat(opcode)
    r_strcat(" ")
    if (dest) {
        r_strcat(dest)
        r_strcat(", ")
    }
    if (operand) {
        r_strcat(operand)
        r_strcat(operand2 ? ", " : "")
    }
    if (operand2) {
        r_strcat(operand2)
    }
    if (comment && is_cflag_enabled("-fverbose-asm")) {
        const char *asm_comment_prefix = get_cflag_value("-fasm-comment");
        if (asm_comment_prefix == NULL)
            asm_comment_prefix = "#";
        r_strcat("\t")
        r_strcat(asm_comment_prefix)
        r_strcat(" ")
        r_strcat(comment)
    }
    r_strcat("\n");
}

typedef struct {
    const char *identifier;
    const char *type;
    int address;
} var_entry_t;

static int cpu_stack_pointer;

size_t var_entry_stack_size = 0;
var_entry_t *var_entry_sp = NULL;

static var_entry_t *push_var(const char *identifier, const char *type)
{
    var_entry_sp = realloc(var_entry_sp, sizeof(var_entry_t) * ++var_entry_stack_size);
    var_entry_sp[var_entry_stack_size - 1].identifier = identifier;
    var_entry_sp[var_entry_stack_size - 1].type = type;
    if (!strcmp(type, "v16"))
        var_entry_sp[var_entry_stack_size - 1].address = --cpu_stack_pointer;
    return var_entry_sp;
}

static var_entry_t *lookup_var(const char *identifier)
{
    for (size_t i = 0; i < var_entry_stack_size; i++)
        if (!strcmp(var_entry_sp[i].identifier, identifier))
            return &var_entry_sp[i];
    abort("lookup_var()", "Could not find the variable \"%s\"", identifier)
}

static ir_register_t ir_registers[] = {
    [0] = {.name = "x0", .busy = false}, // zero
    [1] = {.name = "x1", .busy = false}, // sp (points to half)
    [2] = {.name = "x2", .busy = false}, // t0
    [3] = {.name = "x3", .busy = false}, // t1
    [4] = {.name = "x4", .busy = false}, // t2
    [5] = {.name = "x5", .busy = false}  // t3
};

static ir_register_t alloc_reg(void)
{
    for (int i = 2; i < (int)(sizeof(ir_registers) / sizeof(ir_registers[0])); i++)
        if (ir_registers[i].busy == false) 
        {
            ir_registers[i].busy = true;
            return ir_registers[i];
        }
    abort("alloc_reg()", "Register allocator could not find a free register")
}

static void free_reg(const char *name)
{
    if (name == NULL) return;
    for (int i = 2; i < (int)(sizeof(ir_registers) / sizeof(ir_registers[0])); i++)
        if (!strcmp(ir_registers[i].name, name) && ir_registers[i].busy == true) {
            ir_registers[i].busy = false;
            return;
        }
    abort("free_reg()", "Failed to free an allocated register. (%s)", name)
}

extern void compiler_warn(const char *message);

[[nodiscard]]
static bool ir_process_function(ast_node_t *fn_node)
{
    if (!fn_node) return false;
    add_label(fn_node->node_union.fn_node.name.ast_nodes[0]->node_union.name_node.value);
    if (!strcmp(fn_node->node_union.fn_node.name.ast_nodes[0]->node_union.name_node.value, "main")) {
        const char *stack_init = get_cflag_value("-fstack-init");
        if (stack_init) {
            emit(ir_inst[IR_LDI], ir_registers[IR_SP].name, stack_init, NULL, "Set up stack pointer");
            cpu_stack_pointer = strtol(stack_init, NULL, 10);
        }
        else {
            compiler_warn("-fstack-init flag was not provided. Defaulting to 65535");
            emit(ir_inst[IR_LDI], ir_registers[IR_SP].name, "65535", NULL, "Set up stack pointer (default)");
            cpu_stack_pointer = 65535;
        }
    }

    ast_node_t *node;
    while ((node = ast_walk(&fn_node->node_union.fn_node.body))) {
        if (node->type == NODE_VARDECL) {
            const char *var_name = node->node_union.vardecl_node.name.ast_nodes[0]->node_union.name_node.value;
            const char *var_type = NULL;
            if (node->node_union.vardecl_node.is_preexisting == false)
                push_var(var_name, var_type = node->node_union.vardecl_node.type.ast_nodes[0]->node_union.name_node.value);
            
            ast_node_t *expr_op;
            for (int i = 0; (expr_op = ast_walk(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.ops)); i++) {
                ast_node_t *expr_operator = ast_walk(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.operator);

                const char *reg_imm = NULL;
                if (i == 0) 
                {
                    if (isdigit(*expr_op->node_union.name_node.value))
                        emit(ir_inst[IR_LDI], reg_imm = alloc_reg().name, expr_op->node_union.name_node.value, NULL, "Load immediate");
                    else {
                        const char *reg_op_var_addr = NULL;
                        emit(ir_inst[IR_LDI], reg_op_var_addr = alloc_reg().name, 
                        itoa(lookup_var(expr_op->node_union.name_node.value)->address), NULL, "Load op var addr");
                        emit(ir_inst[IR_LH], reg_imm = alloc_reg().name, reg_op_var_addr, NULL, "Load op var");
                        free_reg(reg_op_var_addr);
                    }
                    /* Save reg_imm onto stack */

                    if (node->node_union.vardecl_node.is_preexisting == false) {
                        const char *reg_sp_subtrahend = NULL;
                        if (!strcmp(var_type, "v16"))
                            emit(ir_inst[IR_LDI], reg_sp_subtrahend = alloc_reg().name, "1", NULL, "Load subtrahend for stack pointer (v16)");
                        emit(ir_inst[IR_SUB], ir_registers[IR_SP].name, ir_registers[IR_SP].name, reg_sp_subtrahend, "Manipulate stack pointer");
                        emit(ir_inst[IR_SH], NULL, ir_registers[IR_SP].name, reg_imm, "Save value onto stack");
                        free_reg(reg_sp_subtrahend);
                    } else {
                        const char *reg_op_var_addr = NULL;
                        emit(ir_inst[IR_LDI], reg_op_var_addr = alloc_reg().name, itoa(lookup_var(var_name)->address), NULL, "Load op var addr");
                        emit(ir_inst[IR_SH], NULL, reg_op_var_addr, reg_imm, "Save value into var address");
                        free_reg(reg_op_var_addr);
                    }
                    free_reg(reg_imm);
                }
                ast_node_t *next_expr_op = ast_peek(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.ops, 1);
                if (next_expr_op == NULL)
                    break;
                
                const char *reg_var_addr = NULL, *reg_var_old_value = NULL;
                if (node->node_union.vardecl_node.is_preexisting == true)
                    emit(ir_inst[IR_LDI], reg_var_addr = alloc_reg().name, itoa(lookup_var(var_name)->address), NULL, "Load var addr");
                
                if (isdigit(*next_expr_op->node_union.name_node.value))
                    emit(ir_inst[IR_LDI], reg_imm = alloc_reg().name, next_expr_op->node_union.name_node.value, NULL, "Load second immediate");
                else {
                    debug_printf("deref: %c\n", *expr_op->node_union.name_node.value);
                    const char *reg_op_var_addr;
                    emit(ir_inst[IR_LDI], reg_op_var_addr = alloc_reg().name, 
                    itoa(lookup_var(expr_op->node_union.name_node.value)->address), NULL, "Load op var addr");
                    emit(ir_inst[IR_LH], reg_imm = alloc_reg().name, reg_op_var_addr, NULL, "Load var");
                    free_reg(reg_op_var_addr);
                }

                emit(ir_inst[IR_LH], reg_var_old_value = alloc_reg().name, 
                node->node_union.vardecl_node.is_preexisting ? reg_var_addr : ir_registers[IR_SP].name, NULL, "Load old var value");
                if (!strcmp(expr_operator->node_union.name_node.value, "+")) {
                    // +
                    emit(ir_inst[IR_ADD], reg_var_old_value, reg_var_old_value, reg_imm, "Add old var value with imm");
                } else if (!strcmp(expr_operator->node_union.name_node.value, "-")) {
                    // -
                    emit(ir_inst[IR_SUB], reg_var_old_value, reg_var_old_value, reg_imm, "Subtract old var value with imm");
                } else if (!strcmp(expr_operator->node_union.name_node.value, "&")) {
                    // bitwise and
                    emit(ir_inst[IR_AND], reg_var_old_value, reg_var_old_value, reg_imm, "AND old var value with imm");
                } else if (!strcmp(expr_operator->node_union.name_node.value, "|")) {
                    // bitwise or
                    emit(ir_inst[IR_OR], reg_var_old_value, reg_var_old_value, reg_imm, "OR old var value with imm");
                } else if (!strcmp(expr_operator->node_union.name_node.value, "^")) {
                    // bitwise xor
                    emit(ir_inst[IR_XOR], reg_var_old_value, reg_var_old_value, reg_imm, "XOR old var value with imm");
                }
                emit(ir_inst[IR_SH], NULL, node->node_union.vardecl_node.is_preexisting ? reg_var_addr : ir_registers[IR_SP].name, reg_var_old_value, "Save the result");

                free_reg(reg_imm);
                free_reg(reg_var_addr);
                free_reg(reg_var_old_value);
            }
        }
    }

    emit("ret", NULL, NULL, NULL, NULL);
    return true;
}

extern ast_node_t *program_node;

void ir_process(void)
{
    while (ir_process_function(ast_walk(&program_node->node_union.program_node.body)));
    ir_state.compiled_len = strlen(ir_state.compiled);
}
