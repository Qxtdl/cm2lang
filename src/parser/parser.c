#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../globals.h"
#include "parser.h"
#include "../lexer/lexer.h"

ast_node_t *program_node = NULL;
ast_node_t *last_created_fn_node = NULL;
ast_node_t *last_created_var_node = NULL;
ast_node_t *last_created_expr_node = NULL;
ast_node_t *last_created_node_eligible_for_block = NULL;
ast_node_t *last_created_node = NULL;
ast_node_ptr_t *last_created_block_nodes = NULL;

static ast_node_t *create_ast_node(bool replace_last_created_node, ast_node_t initalizer_node)
{
    ast_node_t *node = calloc(1, sizeof(ast_node_t));
    memcpy(node, &initalizer_node, sizeof(ast_node_t));
    if (replace_last_created_node) {
        last_created_node = node;
    }
    switch (initalizer_node.type) {
    case NODE_PROGRAM:
        program_node = node;
        break;
    case NODE_FUNCTION:
        last_created_fn_node = node, last_created_node_eligible_for_block = node;
        break;
    case NODE_VARDECL:
        last_created_var_node = node;
        break;
    case NODE_EXPRESSION:
        last_created_expr_node = node;
        break;
    }
    return node;
}
    
static void insert_ast_node(ast_node_ptr_t *ptr, ast_node_t *new_node)
{
    ptr->ast_nodes = realloc(ptr->ast_nodes, ++(ptr->num_children) * sizeof(ast_node_t *));
    ptr->ast_nodes[ptr->num_children - 1] = new_node;
}

ast_node_t *ast_walk(ast_node_ptr_t *ptr)
{
    if (ptr->times_walked < ptr->num_children)
        return ptr->ast_nodes[ptr->times_walked++];
    // Walked too many times
    return NULL;
}

ast_node_t *ast_peek(ast_node_ptr_t *ptr, int offset)
{
    if (ptr->times_walked + offset - 1 < ptr->num_children)
        return ptr->ast_nodes[ptr->times_walked + offset - 1];
    return NULL;
}

typedef enum {
    PARSE_GLOBAL,           // 0
    PARSE_FUNCTION,         // 1
    PARSE_FUNCTION_PARAMS,  // 2
    PARSE_BLOCK,            // 3
    PARSE_VAR,              // 4
    PARSE_VAR_INIT          // 5
} parser_context_t;
    
size_t context_stack_size = 0;
parser_context_t *context_sp = NULL;
parser_context_t current_context;

static void push_context(parser_context_t context)
{
    context_sp = realloc(context_sp, sizeof(parser_context_t) * (context_stack_size + 1));
    context_sp[context_stack_size++] = context;
    current_context = context;
}

static parser_context_t pop_context(void)
{
    parser_context_t popped = context_sp[--context_stack_size - 1];
    context_sp = realloc(context_sp, sizeof(parser_context_t) * context_stack_size);
    current_context = popped;
    return popped;
}

static parser_context_t pop_n_context(size_t n)
{
    parser_context_t popped = 0;
    for (size_t i = 0; i < n; i++) {
        popped = context_sp[--context_stack_size - 1];
        context_sp = realloc(context_sp, sizeof(parser_context_t) * context_stack_size);
        current_context = popped;
    }
    return popped;
}

void parser_process(void)
{   
    push_context(PARSE_GLOBAL);
    create_ast_node(true, (ast_node_t){NODE_PROGRAM, {.program_node = {{0}}}});
    
    bool parser_continue = true;
    while (parser_continue)
    {
        token_t token = lexer_read_token(&parser_continue);
        if (!parser_continue) break;

        switch (token.type) {

        // TODO: Remove {0} initalizers and don't use them instead. See if it works...
        case TOKEN_FN:
            push_context(PARSE_FUNCTION);
            insert_ast_node(&program_node->node_union.program_node.body, 
                create_ast_node(true, (ast_node_t){NODE_FUNCTION, {.fn_node = {{0}}}})); 
            break;
        case TOKEN_L_BRACE:
            push_context(PARSE_BLOCK);
            last_created_block_nodes = &last_created_node_eligible_for_block->node_union.fn_node.body; // TODO: REMOVE, VERY TEMPORARY
            break;
        case TOKEN_R_BRACE: pop_context(); break;
        case TOKEN_NORETURN:
            insert_ast_node(&last_created_fn_node->node_union.fn_node.return_type, 
                create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {"noreturn"}}}));
            break;
        case TOKEN_NOPARAM:
            insert_ast_node(&last_created_fn_node->node_union.fn_node.params, 
                create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {"noparam"}}}));
            pop_n_context(2);
            break;
        case TOKEN_IF:
            debug_printf("Got TOKEN_IF\n");
            break;
        case TOKEN_ASM:
            debug_printf("Got TOKEN_ASM\n");
            break;            
        case TOKEN_V16:
        case TOKEN_V32:
            switch (current_context) {
            case PARSE_FUNCTION:
                insert_ast_node(&last_created_fn_node->node_union.fn_node.return_type, 
                    create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
                break;
            case PARSE_FUNCTION_PARAMS:
                insert_ast_node(&last_created_fn_node->node_union.fn_node.params,
                    create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
                pop_context();
                break;
            case PARSE_BLOCK:
                insert_ast_node(last_created_block_nodes, 
                    create_ast_node(true, (ast_node_t){NODE_VARDECL, {.vardecl_node = {0}}}));
                insert_ast_node(&last_created_node->node_union.vardecl_node.type, 
                    create_ast_node(false, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
                push_context(PARSE_VAR);
                break;
            default: app_abort("parser_process()", "default hit in TOKEN_VARIABLE")
            }
            break;
        case TOKEN_NAME:
            switch(current_context) {
            case PARSE_FUNCTION:
                push_context(PARSE_FUNCTION_PARAMS);
                insert_ast_node(&last_created_fn_node->node_union.fn_node.name,
                    create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));                      
                break;
            case PARSE_VAR:
                insert_ast_node(&last_created_node->node_union.vardecl_node.name,
                    create_ast_node(false, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
                break;
            case PARSE_VAR_INIT:
                insert_ast_node(&last_created_expr_node->node_union.expr_node.ops,
                    create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
                break;
            case PARSE_BLOCK:
                insert_ast_node(&last_created_node_eligible_for_block->node_union.fn_node.body, 
                    create_ast_node(true, (ast_node_t){NODE_VARDECL, {.vardecl_node = {0}}}));
                insert_ast_node(&last_created_var_node->node_union.vardecl_node.name, 
                    create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
                last_created_var_node->node_union.vardecl_node.is_preexisting = true;
                push_context(PARSE_VAR);
                break;
            default: app_abort("parser_process()", "default hit in TOKEN_NAME")
            }
            break;
        case TOKEN_ASSIGN:
            // TODO: Not all vars have to be initalized
            push_context(PARSE_VAR_INIT);
            insert_ast_node(&last_created_var_node->node_union.vardecl_node.init, 
                create_ast_node(true, (ast_node_t){NODE_EXPRESSION, {.expr_node = {{0}}}}));
            break;
        case TOKEN_NUMBER:
            insert_ast_node(&last_created_expr_node->node_union.expr_node.ops, 
                create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
            break;
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_BITWISE_AND:
        case TOKEN_BITWISE_OR:
        case TOKEN_BITWISE_XOR:
            insert_ast_node(&last_created_expr_node->node_union.expr_node.operator, 
                create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
            break;
        case TOKEN_SEMICOLON:
            switch (current_context) {
            case PARSE_VAR_INIT: pop_n_context(2); break;
            default: app_abort("parser_process()", "default hit in TOKEN_SEMICOLON")
            }
            break;
        }
    }
}
