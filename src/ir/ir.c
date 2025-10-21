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
    r_strcat("\t");
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
    if (comment && is_cflag_enabled("-fverbose-asm")) {
        r_strcat("\t# ");
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
    //DBG//
    debug_printf("pushed %s\n", identifier);
    //DBG//
    return var_entry_sp;
}

static var_entry_t *lookup_var(const char *identifier)
{
    for (size_t i = 0; i < var_entry_stack_size; i++)
        if (!strcmp(var_entry_sp[i].identifier, identifier))
            return &var_entry_sp[i];
    abort("lookup_var()", "Could not find the variable \"%s\"", identifier)
}

static int num_alloced_regs = 0;

/* URGENT TODO Full refactor
 *
 * Calling alloc then free or n_free is inefficient,
 * instead make it allocate and free whichever one is available
 * instead of doing it in order.
*/
static const char *alloc_reg(void)
{
    const char *index = ir_registers[2 + num_alloced_regs++];
    if (2 + num_alloced_regs - 1 > sizeof(ir_registers) / sizeof(ir_registers[0]))
        // TODO: FIXME
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

extern void compiler_warn(const char *message);

static bool ir_process_function(ast_node_t *fn_node)
{
    if (!fn_node) return false;
    add_label(fn_node->node_union.fn_node.name.ast_nodes[0]->node_union.name_node.value);
    if (!strcmp(fn_node->node_union.fn_node.name.ast_nodes[0]->node_union.name_node.value, "main")) {
        const char *stack_init = get_cflag_value("-fstack-init");
        if (stack_init) {
            emit(ir_inst[IR_LDI], ir_registers[IR_SP], stack_init, NULL, "Set up stack pointer");
            cpu_stack_pointer = strtol(stack_init, NULL, 10);
        }
        else {
            compiler_warn("-fstack-init flag was not provided. Defaulting to 65535");
            emit(ir_inst[IR_LDI], ir_registers[IR_SP], "65535", NULL, "Set up stack pointer (default)");
            cpu_stack_pointer = 65535;
        }
    }

    ast_node_t *node;
    while (node = ast_walk(&fn_node->node_union.fn_node.body)) {
        if (node->type == NODE_VARDECL) {
            const char *var_name = node->node_union.vardecl_node.name.ast_nodes[0]->node_union.name_node.value;
            const char *var_type = NULL;
            if (node->node_union.vardecl_node.is_preexisting == false)
                push_var(var_name, var_type = node->node_union.vardecl_node.type.ast_nodes[0]->node_union.name_node.value);
            
            ast_node_t *expr_op;
            for (int i = 0; expr_op = ast_walk(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.ops); i++) {
                ast_node_t *expr_operator = ast_walk(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.operator);
                char *reg_imm, *sp_subtrahend;
                if (i == 0) {
                    if (isdigit(*expr_op->node_union.name_node.value))
                        emit(ir_inst[IR_LDI], reg_imm = alloc_reg(), expr_op->node_union.name_node.value, NULL, "Load immediate");
                    else {
                        char *op_var_addr;
                        emit(ir_inst[IR_LDI], op_var_addr = alloc_reg(), 
                        itoa(lookup_var(expr_op->node_union.name_node.value)->address), NULL, "Load op var addr");
                        emit(ir_inst[IR_LH], reg_imm = alloc_reg(), op_var_addr, NULL, "Load var");
                        free_reg();          // it is bad to do it
                        reg_imm = alloc_reg();  // like this bloaty
                    }
                    if (node->node_union.vardecl_node.is_preexisting == false) {
                        if (!strcmp(var_type, "v16"))
                            emit(ir_inst[IR_LDI], sp_subtrahend = alloc_reg(), "1", NULL, "Load subtrahend for stack pointer (v16)");
                        emit(ir_inst[IR_SUB], ir_registers[IR_SP], ir_registers[IR_SP], sp_subtrahend, "Manipulate stack pointer");
                        emit(ir_inst[IR_SH], NULL, ir_registers[IR_SP], reg_imm, "Save value onto stack");
                        n_free_reg(2);
                    } else {
                        char *var_addr;
                        if (node->node_union.vardecl_node.is_preexisting == true)
                            emit(ir_inst[IR_LDI], var_addr = alloc_reg(), itoa(lookup_var(var_name)->address), NULL, "Load var addr");
                        if (!isdigit(*expr_op->node_union.name_node.value)) 
                            emit(ir_inst[IR_LH], reg_imm = alloc_reg(), var_addr, NULL, "Load var");
                        emit(ir_inst[IR_SH], NULL, var_addr, reg_imm, "Save value into var address");
                        n_free_reg(2);                        
                    }
                }
                ast_node_t *next_expr_op = ast_peek(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.ops, 1);
                if (next_expr_op == NULL)
                    break;
                /* We have an operand to process */
                
                char *var_old_value, *var_addr;
                if (node->node_union.vardecl_node.is_preexisting == true)
                    emit(ir_inst[IR_LDI], var_addr = alloc_reg(), itoa(lookup_var(var_name)->address), NULL, "Load var addr");
                if (isdigit(*expr_op->node_union.name_node.value))
                    emit(ir_inst[IR_LDI], reg_imm = alloc_reg(), next_expr_op->node_union.name_node.value, NULL, "Load second immediate");
                else {
                    // TODO: Need to get the var and load it into reg_imm
                    emit(ir_inst[IR_LH], reg_imm = alloc_reg(), var_addr, NULL, "Load second var");
                }

                emit(ir_inst[IR_LH], var_old_value = alloc_reg(), 
                node->node_union.vardecl_node.is_preexisting ? var_addr : ir_registers[IR_SP], NULL, "Load old var value");
                if (!strcmp(expr_operator->node_union.name_node.value, "+")) {
                    // +
                    emit(ir_inst[IR_ADD], var_old_value, var_old_value, reg_imm, "Add old var value with imm");
                } else if (!strcmp(expr_operator->node_union.name_node.value, "-")) {
                    // -
                    emit(ir_inst[IR_SUB], var_old_value, var_old_value, reg_imm, "Subtract old var value with imm");
                } else if (!strcmp(expr_operator->node_union.name_node.value, "&")) {
                    // bitwise and
                    emit(ir_inst[IR_AND], var_old_value, var_old_value, reg_imm, "AND old var value with imm");
                } else if (!strcmp(expr_operator->node_union.name_node.value, "|")) {
                    // bitwise or
                    emit(ir_inst[IR_OR], var_old_value, var_old_value, reg_imm, "OR old var value with imm");
                } else if (!strcmp(expr_operator->node_union.name_node.value, "^")) {
                    // bitwise xor
                    emit(ir_inst[IR_XOR], var_old_value, var_old_value, reg_imm, "XOR old var value with imm");
                }
                emit(ir_inst[IR_SH], NULL, node->node_union.vardecl_node.is_preexisting ? var_addr : ir_registers[IR_SP], var_old_value, "Save the result");

                if (node->node_union.vardecl_node.is_preexisting == true)
                    n_free_reg(3);
                else
                    free_reg();
            }

            // After var value is processed
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
