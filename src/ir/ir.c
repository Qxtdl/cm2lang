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

struct ir_state ir_state = {0};

void ir_init(void)
{
    ir_state.compiled = malloc(1);
    *ir_state.compiled = '\0';
    ir_state.compiled_len = 0;
}

#define ir_rstrcat(src) \
ir_state.compiled = realloc(ir_state.compiled, strlen(ir_state.compiled) + strlen(src) + 1); \
strcat(ir_state.compiled, src);

static void add_label(const char *name)
{
    ir_rstrcat(name);
    ir_rstrcat(":\n");
}

static size_t label_id = 0;

static const char *add_id_label(const char *name)
{
    ir_rstrcat(name);
    ir_rstrcat(itoa(label_id)); // weridly label_id++ doesn't work
    label_id++;
    ir_rstrcat(":\n")
}

static const char *alloc_id_label(const char *name)
{
    char *label = strdup(name);
    label = r_strcat(label, itoa(label_id));
    label_id++;
}

static void emit(
    const char *opcode, 
    const char *dest,
    const char *operand,
    const char *operand2,
    const char *operand3,
    const char *comment
)
{
    ir_rstrcat("\t")
    ir_rstrcat(opcode)
    ir_rstrcat(" ")
    if (dest) {
        ir_rstrcat(dest)
        ir_rstrcat(", ")
    }
    if (operand) {
        ir_rstrcat(operand)
        ir_rstrcat(operand2 ? ", " : "")
    }
    if (operand2) {
        ir_rstrcat(operand2)
        ir_rstrcat(operand3 ? ", " : "")        
    }
    if (operand3) {
        ir_rstrcat(operand3)
    }
    if (comment && is_cflag_enabled(cflags[FLAG_VERBOSE_ASM])) {
        const char *asm_comment_prefix = get_cflag_value(cflags[FLAG_ASM_COMMENT]);
        if (asm_comment_prefix == NULL)
            asm_comment_prefix = "#";
        ir_rstrcat("\t")
        ir_rstrcat(asm_comment_prefix)
        ir_rstrcat(" ")
        ir_rstrcat(comment)
    }
    ir_rstrcat("\n");
}

typedef struct {
    const char *identifier;
    const char *type;
    int address;
} var_entry_t;

size_t var_entry_stack_size = 0;
var_entry_t *var_entry_sp = NULL;

static int cpu_stack_pointer;

static var_entry_t *push_var(const char *identifier, const char *type)
{
    var_entry_sp = realloc(var_entry_sp, sizeof(var_entry_t) * ++var_entry_stack_size);
    var_entry_sp[var_entry_stack_size - 1].identifier = identifier;
    var_entry_sp[var_entry_stack_size - 1].type = type;
    if (!strcmp(type, "v16"))
        var_entry_sp[var_entry_stack_size - 1].address = cpu_stack_pointer = cpu_stack_pointer - strtol(get_cflag_value(cflags[FLAG_HALF_SIZE]), NULL, 10);
    return var_entry_sp;
}

static var_entry_t *lookup_var(const char *identifier)
{
    for (size_t i = 0; i < var_entry_stack_size; i++)
        if (!strcmp(var_entry_sp[i].identifier, identifier))
            return &var_entry_sp[i];
    app_abort("lookup_var()", "Could not find the variable \"%s\"", identifier)
}

static ir_register_t ir_registers[] = {
    [0] = {.name = "x0", .busy = true}, // zero (true just incase so it can never be used?)
    [1] = {.name = "x1", .busy = true}, // sp (points to half) (true just incase so it can be never used?)
    [2] = {.name = "x2", .busy = false}, // t0
    [3] = {.name = "x3", .busy = false}, // t1
    [4] = {.name = "x4", .busy = false}, // t2
    [5] = {.name = "x5", .busy = false}  // t3
};

static const char *alloc_reg(void)
{
    for (int i = 2; i < (int)(sizeof(ir_registers) / sizeof(ir_registers[0])); i++)
        if (ir_registers[i].busy == false) {
            ir_registers[i].busy = true;
            return ir_registers[i].name;
        }
    app_abort("alloc_reg()", "Register allocator could not find a free register. The compiler is leaking registers!")
}

static void free_reg(const char *name)
{
    if (name == NULL) return;
    for (int i = 2; i < (int)(sizeof(ir_registers) / sizeof(ir_registers[0])); i++)
        if (!strcmp(ir_registers[i].name, name) && ir_registers[i].busy == true) {
            ir_registers[i].busy = false;
            return;
        }
    app_abort("free_reg()", "Failed to free an allocated register. (%s)", name)
}

extern const char *const tokens[];

static void eval_arith_operator(const char *operator, const char *reg_rd, const char *reg_rs1, const char *reg_rs2)
{
    if (!strcmp(operator, tokens[TOKEN_PLUS])) {
        // +
        emit(ir_inst[IR_ADD], reg_rd, reg_rs1, reg_rs2, NULL, "Perform addition");
    } else if (!strcmp(operator, tokens[TOKEN_MINUS])) {
        // -
        emit(ir_inst[IR_SUB], reg_rd, reg_rs1, reg_rs2, NULL, "Perform subtraction");
    } else if (!strcmp(operator, tokens[TOKEN_BITWISE_AND])) {
        // bitwise and
        emit(ir_inst[IR_AND], reg_rd, reg_rs1, reg_rs2, NULL, "Perform AND");
    } else if (!strcmp(operator, tokens[TOKEN_BITWISE_OR])) {
        // bitwise or
        emit(ir_inst[IR_OR], reg_rd, reg_rs1, reg_rs2, NULL, "Perform OR");
    } else if (!strcmp(operator, tokens[TOKEN_BITWISE_XOR])) {  
        // bitwise xor
        emit(ir_inst[IR_XOR], reg_rd, reg_rs1, reg_rs2, NULL, "Perform XOR");
    } else if (!strcmp(operator, tokens[TOKEN_EQUALS]) || !strcmp(operator, tokens[TOKEN_NOT_EQUALS])) {
        // conditional operator
        emit(ir_inst[IR_ADD], reg_rd, reg_rs2, ir_registers[IR_ZERO].name, NULL, "Conditional operator");
    }
}

#define emit_push(reg) \
{ \
    const char *reg_sp_subtrahend = alloc_reg(); \
    emit(ir_inst[IR_LDI], reg_sp_subtrahend, get_cflag_value(cflags[FLAG_HALF_SIZE]), NULL, NULL, "Load subtrahend for stack pointer"); \
    emit(ir_inst[IR_SUB], ir_registers[IR_SP].name, ir_registers[IR_SP].name, reg_sp_subtrahend, NULL, "Manipulate stack pointer"); \
    emit(ir_inst[IR_SH], NULL, ir_registers[IR_SP].name, reg, NULL, "Save value onto stack"); \
    free_reg(reg_sp_subtrahend); \
}

#define emit_pop(reg) \
{ \
    const char *reg_sp_addend = alloc_reg(); \
    emit(ir_inst[IR_LH], reg, ir_registers[IR_SP].name, NULL, NULL, "Pop off the stack"); \
    emit(ir_inst[IR_LDI], reg_sp_addend, get_cflag_value(cflags[FLAG_HALF_SIZE]), NULL, NULL, "Load addend for stack pointer"); \
    emit(ir_inst[IR_ADD], ir_registers[IR_SP].name, ir_registers[IR_SP].name, reg_sp_addend, NULL, "Manipulate stack pointer"); \
    free_reg(reg_sp_addend); \
}

static void ir_process_scope(ast_node_ptr_t *body)
{
    ast_node_t *node;
    while ((node = ast_walk(body))) {
        if (node->type == NODE_VARDECL) {
            const char *var_name = node->node_union.vardecl_node.name.ast_nodes[0]->node_union.name_node.value;
            const char *var_type = NULL;
            if (node->node_union.vardecl_node.is_preexisting == false)
                push_var(var_name, var_type = node->node_union.vardecl_node.type.ast_nodes[0]->node_union.name_node.value);
            
            ast_node_t *expr_op;
            for (size_t i = 0; (expr_op = ast_walk(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.ops)); i++) {
                ast_node_t *expr_operator = ast_walk(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.operator);

                const char *reg_imm = NULL;
                if (i == 0) 
                {
                    if (isdigit(*expr_op->node_union.name_node.value))
                        emit(ir_inst[IR_LDI], reg_imm = alloc_reg(), expr_op->node_union.name_node.value, NULL, NULL, "Load immediate");
                    else {
                        const char *reg_op_var_addr = NULL;
                        emit(ir_inst[IR_LDI], reg_op_var_addr = alloc_reg(), 
                        itoa(lookup_var(expr_op->node_union.name_node.value)->address), NULL, NULL, "Load op var addr");
                        emit(ir_inst[IR_LH], reg_imm = alloc_reg(), reg_op_var_addr, NULL, NULL, "Load op var");
                        free_reg(reg_op_var_addr);
                    }
                    /* Save reg_imm onto stack */

                    if (node->node_union.vardecl_node.is_preexisting == false) {
                        const char *reg_sp_subtrahend = NULL;
                        if (!strcmp(var_type, "v16"))
                            emit(ir_inst[IR_LDI], reg_sp_subtrahend = alloc_reg(), get_cflag_value(cflags[FLAG_HALF_SIZE]), NULL, NULL, "Load subtrahend for stack pointer (v16)");
                        emit(ir_inst[IR_SUB], ir_registers[IR_SP].name, ir_registers[IR_SP].name, reg_sp_subtrahend, NULL, "Manipulate stack pointer");
                        emit(ir_inst[IR_SH], NULL, ir_registers[IR_SP].name, reg_imm, NULL, "Save value onto stack");
                        free_reg(reg_sp_subtrahend);
                    } else {
                        const char *reg_op_var_addr = NULL;
                        emit(ir_inst[IR_LDI], reg_op_var_addr = alloc_reg(), itoa(lookup_var(var_name)->address), NULL, NULL, "Load op var addr");
                        emit(ir_inst[IR_SH], NULL, reg_op_var_addr, reg_imm, NULL, "Save value into var address");
                        free_reg(reg_op_var_addr);
                    }
                    free_reg(reg_imm);
                }
                ast_node_t *next_expr_op = ast_peek(&node->node_union.vardecl_node.init.ast_nodes[0]->node_union.expr_node.ops, 1);
                if (next_expr_op == NULL)
                    break;
                
                const char *reg_var_addr = NULL, *reg_var_old_value = NULL;
                if (node->node_union.vardecl_node.is_preexisting == true)
                    emit(ir_inst[IR_LDI], reg_var_addr = alloc_reg(), itoa(lookup_var(var_name)->address), NULL, NULL, "Load var addr");
                
                if (isdigit(*next_expr_op->node_union.name_node.value))
                    emit(ir_inst[IR_LDI], reg_imm = alloc_reg(), next_expr_op->node_union.name_node.value, NULL, NULL, "Load second immediate");
                else {
                    const char *reg_op_var_addr;
                    emit(ir_inst[IR_LDI], reg_op_var_addr = alloc_reg(), 
                    itoa(lookup_var(next_expr_op->node_union.name_node.value)->address), NULL, NULL, "Load op var addr");
                    emit(ir_inst[IR_LH], reg_imm = alloc_reg(), reg_op_var_addr, NULL, NULL, "Load var");
                    free_reg(reg_op_var_addr);
                }
                emit(ir_inst[IR_LH], reg_var_old_value = alloc_reg(), 
                node->node_union.vardecl_node.is_preexisting ? reg_var_addr : ir_registers[IR_SP].name, NULL, NULL, "Load old var value");
                eval_arith_operator(expr_operator->node_union.name_node.value, reg_var_old_value, reg_var_old_value, reg_imm);
                emit(ir_inst[IR_SH], NULL, node->node_union.vardecl_node.is_preexisting ? reg_var_addr : ir_registers[IR_SP].name, reg_var_old_value, NULL, "Save the result");

                free_reg(reg_imm);
                free_reg(reg_var_addr);
                free_reg(reg_var_old_value);
            }
        }
        else if (node->type == NODE_INLINE_ASM) {
            ir_rstrcat("\t")
            ir_rstrcat(node->node_union.inline_asm_node.inline_asm.ast_nodes[0]->node_union.name_node.value)
            ir_rstrcat("\n")
        }
        else if (node->type == NODE_IF_STATEMENT) {
            const char *reg_value_to_compare = NULL, *reg_imm = NULL;
            token_type_t comparison_operator = TOKEN_NULL;

            ast_node_t *expr_op;
            for (size_t i = 0; (expr_op = ast_walk(&node->node_union.if_statement_node.condition.ast_nodes[0]->node_union.expr_node.ops)); i++) {
                ast_node_t *expr_operator = ast_walk(&node->node_union.if_statement_node.condition.ast_nodes[0]->node_union.expr_node.operator);
                if (i == 0)
                {
                    if (isdigit(*expr_op->node_union.name_node.value))
                        if (!strcmp(expr_operator->node_union.name_node.value, tokens[TOKEN_EQUALS]) || !strcmp(expr_operator->node_union.name_node.value, tokens[TOKEN_NOT_EQUALS]))
                            emit(ir_inst[IR_LDI], reg_imm = alloc_reg(), expr_op->node_union.name_node.value, NULL, NULL, "Load immediate to check");
                        else
                            emit(ir_inst[IR_LDI], reg_imm = alloc_reg(), expr_op->node_union.name_node.value, NULL, NULL, "Load immediate to process");
                    else {
                        const char *op_var_addr = alloc_reg();
                        if (!strcmp(expr_operator->node_union.name_node.value, tokens[TOKEN_EQUALS]) || !strcmp(expr_operator->node_union.name_node.value, tokens[TOKEN_NOT_EQUALS])) {
                            emit(ir_inst[IR_LDI], op_var_addr, itoa(lookup_var(expr_op->node_union.name_node.value)->address), NULL, NULL, "Load var addr to check");
                            emit(ir_inst[IR_LH], reg_imm = alloc_reg(), op_var_addr, NULL, NULL, "Load var to check");
                        }
                        else {
                            emit(ir_inst[IR_LDI], op_var_addr, itoa(lookup_var(expr_op->node_union.name_node.value)->address), NULL, NULL, "Load var addr to process");
                            emit(ir_inst[IR_LH], reg_imm = alloc_reg(), op_var_addr, NULL, NULL, "Load var to process");
                        }
                        free_reg(op_var_addr);
                    }
                }
                // Do we have operator? Update comparison_operator
                if (expr_operator && !strcmp(expr_operator->node_union.name_node.value, tokens[TOKEN_EQUALS])) comparison_operator = TOKEN_EQUALS;
                if (expr_operator && !strcmp(expr_operator->node_union.name_node.value, tokens[TOKEN_NOT_EQUALS])) comparison_operator = TOKEN_NOT_EQUALS;

                ast_node_t *next_expr_op = ast_peek(&node->node_union.if_statement_node.condition.ast_nodes[0]->node_union.expr_node.ops, 1);
                if (next_expr_op == NULL) {
                    // Handle dealing with evaluating the if statement
                    const char *label_body = alloc_id_label("if_true"), *label_skip_if = alloc_id_label("skip_if");
                    if (comparison_operator == TOKEN_EQUALS)
                        emit(ir_inst[IR_BEQ], NULL, reg_value_to_compare, reg_imm, label_body, "Evaluate if statement");
                    else {
                        const char *label_skip_not_true = alloc_id_label("skip_not_true");
                        emit(ir_inst[IR_BEQ], NULL, reg_value_to_compare, reg_imm, label_skip_not_true, "Evaluate if statement");
                        emit(ir_inst[IR_JMP], NULL, NULL, NULL, label_body, "Evaluate if statement");
                        add_label(label_skip_not_true);
                    }
                    emit(ir_inst[IR_JMP], NULL, label_skip_if, NULL, NULL, "If false, skip if");
                    add_label(label_body);
                    free_reg(reg_value_to_compare);
                    free_reg(reg_imm);
                    ir_process_scope(&node->node_union.if_statement_node.body);
                    add_label(label_skip_if);
                    break;
                }
                const char *reg_op = alloc_reg();
                if (comparison_operator == TOKEN_NULL) {
                    // We are still evaluating the value to check
                    if (isdigit(*next_expr_op->node_union.name_node.value))
                        emit(ir_inst[IR_LDI], reg_op, next_expr_op->node_union.name_node.value, NULL, NULL, "Load op imm");
                    else {
                        const char *reg_op_addr = alloc_reg();
                        emit(ir_inst[IR_LDI], reg_op_addr, itoa(lookup_var(next_expr_op->node_union.name_node.value)->address), NULL, NULL, "Load op var addr");
                        emit(ir_inst[IR_LH], reg_op, reg_op_addr, NULL, NULL, "Load op var");
                        free_reg(reg_op_addr);
                    }
                    eval_arith_operator(expr_operator->node_union.name_node.value, reg_imm, reg_imm, reg_op);
                }
                else if (comparison_operator == TOKEN_EQUALS || comparison_operator == TOKEN_NOT_EQUALS) {
                    if (isdigit(*next_expr_op->node_union.name_node.value))
                        emit(ir_inst[IR_LDI], reg_op, next_expr_op->node_union.name_node.value, NULL, NULL, "Load op imm");
                    else {
                        const char *reg_op_addr = alloc_reg();
                        emit(ir_inst[IR_LDI], reg_op_addr, itoa(lookup_var(next_expr_op->node_union.name_node.value)->address), NULL, NULL, "Load op var addr");
                        emit(ir_inst[IR_LH], reg_op, reg_op_addr, NULL, NULL, "Load op var");
                        free_reg(reg_op_addr);
                    }
                    if (reg_value_to_compare == NULL) reg_value_to_compare = alloc_reg();
                    eval_arith_operator(expr_operator->node_union.name_node.value, reg_value_to_compare, reg_value_to_compare, reg_op);
                }
                free_reg(reg_op);
            }
        }
        else if (node->type == NODE_WHILE_STATEMENT) {
            /* TODO
             * This code is very broken and segfaults and reused code from if statement.
             * This design also won't work since reg_value_to_compare and reg_imm are stored in registers then checked,
             * in turn if you generate a scope there will be 2 registers missing, and if you have nested while loops it will cause a register leak
             * In short this is a design flaw and needs to be fixed, but atleast if statement is working!
             */  
            const char *reg_value_to_compare = NULL, *reg_imm = NULL;
            token_type_t comparison_operator = TOKEN_NULL;

            ast_node_t *expr_op;
            for (size_t i = 0; (expr_op = ast_walk(&node->node_union.while_statement_node.condition.ast_nodes[0]->node_union.expr_node.ops)); i++) {
                ast_node_t *expr_operator = ast_walk(&node->node_union.while_statement_node.condition.ast_nodes[0]->node_union.expr_node.operator);
                if (i == 0)
                {
                    if (isdigit(*expr_op->node_union.name_node.value))
                        if (!strcmp(expr_operator->node_union.name_node.value, tokens[TOKEN_EQUALS]) || !strcmp(expr_operator->node_union.name_node.value, tokens[TOKEN_NOT_EQUALS]))
                            emit(ir_inst[IR_LDI], reg_imm = alloc_reg(), expr_op->node_union.name_node.value, NULL, NULL, "Load immediate to check");
                        else
                            emit(ir_inst[IR_LDI], reg_imm = alloc_reg(), expr_op->node_union.name_node.value, NULL, NULL, "Load immediate to process");
                    else {
                        const char *op_var_addr = alloc_reg();
                        if (!strcmp(expr_operator->node_union.name_node.value, tokens[TOKEN_EQUALS]) || !strcmp(expr_operator->node_union.name_node.value, tokens[TOKEN_NOT_EQUALS])) {
                            emit(ir_inst[IR_LDI], op_var_addr, itoa(lookup_var(expr_op->node_union.name_node.value)->address), NULL, NULL, "Load var addr to check");
                            emit(ir_inst[IR_LH], reg_imm = alloc_reg(), op_var_addr, NULL, NULL, "Load var to check");
                        }
                        else {
                            emit(ir_inst[IR_LDI], op_var_addr, itoa(lookup_var(expr_op->node_union.name_node.value)->address), NULL, NULL, "Load var addr to process");
                            emit(ir_inst[IR_LH], reg_imm = alloc_reg(), op_var_addr, NULL, NULL, "Load var to process");
                        }
                        free_reg(op_var_addr);
                    }
                }
                // Do we have operator? Update comparison_operator
                if (expr_operator && !strcmp(expr_operator->node_union.name_node.value, tokens[TOKEN_EQUALS])) comparison_operator = TOKEN_EQUALS;
                if (expr_operator && !strcmp(expr_operator->node_union.name_node.value, tokens[TOKEN_NOT_EQUALS])) comparison_operator = TOKEN_NOT_EQUALS;

                ast_node_t *next_expr_op = ast_peek(&node->node_union.while_statement_node.condition.ast_nodes[0]->node_union.expr_node.ops, 1);
                if (next_expr_op == NULL) {
                    // Check reg_value_to_check and reg_imm
                    const char *label_body = alloc_id_label("while_true"), *label_skip = alloc_id_label("skip_while");
                    if (comparison_operator == TOKEN_EQUALS) {
                        emit(ir_inst[IR_BEQ], NULL, reg_value_to_compare, reg_imm, label_body);
                    }
                }
                const char *reg_op = alloc_reg();
                if (comparison_operator == TOKEN_NULL) {
                    // We are still evaluating the value to check
                    if (isdigit(*next_expr_op->node_union.name_node.value))
                        emit(ir_inst[IR_LDI], reg_op, next_expr_op->node_union.name_node.value, NULL, NULL, "Load op imm");
                    else {
                        const char *reg_op_addr = alloc_reg();
                        emit(ir_inst[IR_LDI], reg_op_addr, itoa(lookup_var(next_expr_op->node_union.name_node.value)->address), NULL, NULL, "Load op var addr");
                        emit(ir_inst[IR_LH], reg_op, reg_op_addr, NULL, NULL, "Load op var");
                        free_reg(reg_op_addr);
                    }
                    eval_arith_operator(expr_operator->node_union.name_node.value, reg_imm, reg_imm, reg_op);
                }
                else if (comparison_operator == TOKEN_EQUALS || comparison_operator == TOKEN_NOT_EQUALS) {
                    if (isdigit(*next_expr_op->node_union.name_node.value))
                        emit(ir_inst[IR_LDI], reg_op, next_expr_op->node_union.name_node.value, NULL, NULL, "Load op imm");
                    else {
                        const char *reg_op_addr = alloc_reg();
                        emit(ir_inst[IR_LDI], reg_op_addr, itoa(lookup_var(next_expr_op->node_union.name_node.value)->address), NULL, NULL, "Load op var addr");
                        emit(ir_inst[IR_LH], reg_op, reg_op_addr, NULL, NULL, "Load op var");
                        free_reg(reg_op_addr);
                    }
                    if (reg_value_to_compare == NULL) reg_value_to_compare = alloc_reg();
                    eval_arith_operator(expr_operator->node_union.name_node.value, reg_value_to_compare, reg_value_to_compare, reg_op);
                }
                free_reg(reg_op);
            }
        }
    }
}

[[nodiscard]]
static bool ir_process_globals(ast_node_t *global_node)
{
    if (!global_node) return false;

    if (global_node->type == NODE_FUNCTION) {
        add_label(global_node->node_union.fn_node.name.ast_nodes[0]->node_union.name_node.value);
        if (!strcmp(global_node->node_union.fn_node.name.ast_nodes[0]->node_union.name_node.value, "main")) {
            // -fno-init-zero-reg
            if (!is_cflag_enabled(cflags[FLAG_NO_INIT_ZERO_REG]))
                emit(ir_inst[IR_LDI], ir_registers[IR_ZERO].name, "0", NULL, NULL, "Init zero reg");
            // -fstack-init
            const char *stack_init = get_cflag_value(cflags[FLAG_STACK_INIT]);
            emit(ir_inst[IR_LDI], ir_registers[IR_SP].name, stack_init, NULL, NULL, "Set up stack pointer");
            cpu_stack_pointer = strtol(stack_init, NULL, 10);
        }
        ir_process_scope(&global_node->node_union.fn_node.body);
        emit(ir_inst[IR_RET], NULL, NULL, NULL, NULL, NULL);
    }
    return true;
}

extern ast_node_t *program_node;

void ir_process(void)
{
    while (ir_process_globals(ast_walk(&program_node->node_union.program_node.body)));
    ir_state.compiled_len = strlen(ir_state.compiled);
}
