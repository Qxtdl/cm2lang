#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../globals.h"
#include "parser.h"
#include "../lexer/lexer.h"

typedef enum {
    PARSE_GLOBAL,
    PARSE_FUNCTION,
    PARSE_FUNCTION_PARAMS,
    PARSE_BLOCK,
    PARSE_VAR,
    PARSE_VAR_INIT
} parser_context_t;

size_t stack_size = 0;
parser_context_t *sp = NULL;
parser_context_t current_context;

static void push_context(parser_context_t context)
{
    sp = realloc(sp, sizeof(parser_context_t) * (stack_size + 1));
    sp[stack_size++] = context;
    current_context = context;
    debug_printf("PUSHED %d SP %d\n\n", context, stack_size);
    //debug_printf("pushed %d\n sp %d\n\n", context, stack_size);
}

static parser_context_t pop_context()
{
    parser_context_t popped = sp[--stack_size - 1];
    sp = realloc(sp, sizeof(parser_context_t) * stack_size);
    current_context = popped;
    debug_printf("POPPED %d SP %d\n\n", popped, stack_size);
    return popped;
}

static parser_context_t pop_n_context(size_t n)
{
    parser_context_t popped;
    for (size_t i = 0; i < n; i++)
    {
        popped = sp[--stack_size - 1];
        sp = realloc(sp, sizeof(parser_context_t) * stack_size);
        current_context = popped;
    }
    debug_printf("N-POPPED %d SP %d\n\n", popped, stack_size);
    return popped;
}

ast_node_t *program_node = NULL;
ast_node_t *last_created_fn_node = NULL;
ast_node_t *last_created_var_node = NULL;
ast_node_t *last_created_expr_node = NULL;
ast_node_t *last_created_node = NULL;
ast_node_ptr_t *last_created_block_nodes = NULL;

static ast_node_t *create_ast_node(bool replace_last_created_node, ast_node_t initalizer_node)
{
    ast_node_t *node = calloc(1, sizeof(ast_node_t));
    memcpy(node, &initalizer_node, sizeof(ast_node_t));
    if (replace_last_created_node)
        last_created_node = node;
    
    switch (initalizer_node.type)
    {
        case NODE_FUNCTION:
            last_created_fn_node = node;
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
    if (ptr->num_children == 0)
        ptr->ast_nodes = malloc(sizeof(ast_node_t *));
    ptr->ast_nodes = realloc(ptr->ast_nodes, ++(ptr->num_children) * sizeof(ast_node_t *));
    ptr->ast_nodes[ptr->num_children - 1] = new_node;
}

ast_node_t *ast_walk(ast_node_ptr_t *ptr)
{
    if (ptr->times_walked < ptr->num_children)
        return ptr->ast_nodes[ptr->times_walked++];
    return NULL; // walked too many times
}

void parser_process(void)
{   
    push_context(PARSE_GLOBAL);
    program_node = create_ast_node(true, (ast_node_t){NODE_PROGRAM, {.program_node = {0}}});

    bool parser_continue = true;
    while (parser_continue)
    {
        token_t token = lexer_read_token(&parser_continue);
        if (!parser_continue) break;

        debug_printf("%s\ncontext:%d\n\n", token.value, current_context);
        switch (token.type)
        {
            case TOKEN_FN:
                push_context(PARSE_FUNCTION);
                insert_ast_node(&program_node->node_union.program_node.body, 
                    create_ast_node(true, (ast_node_t){NODE_FUNCTION, {.fn_node = {0}}})); 
                break;
            case TOKEN_L_BRACE:
                push_context(PARSE_BLOCK);
                last_created_block_nodes = &last_created_fn_node->node_union.fn_node.body; // TODO: REMOVE, VERY TEMPORARY
                break;
            case TOKEN_R_BRACE:
                pop_context(); break;
            case TOKEN_NORETURN:
                insert_ast_node(&last_created_fn_node->node_union.fn_node.return_type, 
                    create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {"noreturn"}}}));
                break;
            case TOKEN_NOPARAM:
                insert_ast_node(&last_created_fn_node->node_union.fn_node.params, 
                    create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {"noparam"}}}));
                pop_n_context(2);
                break;
            case TOKEN_U8:
            case TOKEN_U16:
            case TOKEN_U32:
            case TOKEN_S8:
            case TOKEN_S16:
            case TOKEN_S32:
                switch (current_context)
                {
                    case PARSE_FUNCTION:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.return_type, create_ast_node(
                            true, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
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
                }
                break;
            case TOKEN_NAME:
                switch(current_context)
                {
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
                }
                break;
            case TOKEN_ASSIGN:
                switch (current_context)
                {
                    case PARSE_VAR:
                        push_context(PARSE_VAR_INIT);
                        insert_ast_node(&last_created_var_node->node_union.vardecl_node.init, 
                            create_ast_node(true, (ast_node_t){NODE_EXPRESSION, {.expr_node = {0}}}));
                        break;
                }
                break;
            case TOKEN_NUMBER:
                switch (current_context)
                {
                    case PARSE_VAR_INIT:
                        insert_ast_node(&last_created_expr_node->node_union.expr_node.ops, 
                            create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
                        break;
                }
                break;
            case TOKEN_PLUS:
            case TOKEN_MINUS:
                switch (current_context)
                {
                    case PARSE_VAR_INIT:
                        insert_ast_node(&last_created_expr_node->node_union.expr_node.operator, 
                            create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
                        break;
                }
                break;
            case TOKEN_SEMICOLON:
                switch (current_context)
                {
                    case PARSE_VAR_INIT:
                        pop_n_context(2);
                        break;
                }
                break;
        }
    }
}
