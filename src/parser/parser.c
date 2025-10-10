#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../globals.h"
#include "parser.h"
#include "../lexer/lexer.h"

enum {
    PARSE_GLOBAL,
    PARSE_FUNCTION,
    PARSE_FUNCTION_PARAMS,
    PARSE_BLOCK
} current_parse_context, before_block_parse_context;

ast_node_t *program_node = NULL;
ast_node_t *last_created_fn_node = NULL;
ast_node_t *last_created_node = NULL;
ast_node_ptr_t *last_created_block_nodes = NULL;

static ast_node_t *create_ast_node(ast_node_t initalizer_node)
{
    ast_node_t *node = calloc(1, sizeof(ast_node_t));
    memcpy(node, &initalizer_node, sizeof(ast_node_t));
    last_created_node = node;
    return node;
}

static void insert_ast_node(ast_node_ptr_t *ptr, ast_node_t *new_node)
{
    if (ptr->num_children == 0)
        ptr->ast_nodes = malloc(sizeof(ast_node_t *));
    
    ptr->ast_nodes = realloc(ptr->ast_nodes, ++(ptr->num_children));
    ptr->ast_nodes[ptr->num_children - 1] = new_node;
}

ast_node_t *ast_walk(ast_node_ptr_t *ptr)
{
    if (ptr->times_walked < ptr->num_children)
        return ptr->ast_nodes[ptr->times_walked++];
    return NULL;
}

void parser_process(void)
{   
    current_parse_context = PARSE_GLOBAL;
    program_node = create_ast_node((ast_node_t){NODE_PROGRAM, {.program_node = {0}}});

    bool parser_continue = true;
    while (parser_continue)
    {
        token_t token = lexer_read_token(&parser_continue);
        if (!parser_continue) break;

        switch (token.type)
        {
            case TOKEN_FN:
                current_parse_context = PARSE_FUNCTION;
                insert_ast_node(&program_node->node_union.program_node.body, (last_created_fn_node = create_ast_node((ast_node_t){NODE_FUNCTION, {.fn_node = {0}}}))); 
                break;
            case TOKEN_L_BRACE:
                before_block_parse_context = current_parse_context;
                current_parse_context = PARSE_BLOCK; 
                last_created_block_nodes = &last_created_fn_node->node_union.fn_node.body; // TODO: REMOVE, VERY TEMPORARY
                break;
            case TOKEN_R_BRACE:
                current_parse_context = before_block_parse_context; break;
            case TOKEN_NORETURN:
                insert_ast_node(&last_created_fn_node->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"noreturn"}}})); break;
            case TOKEN_NOPARAM:
                insert_ast_node(&last_created_fn_node->node_union.fn_node.params, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"noparam"}}})); break;
            case TOKEN_U8:
                switch (current_parse_context)
                {
                    case PARSE_FUNCTION:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"u8"}}})); break;
                    case PARSE_FUNCTION_PARAMS:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.params,create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"u8"}}})); break;
                    case PARSE_BLOCK:
                        insert_ast_node(last_created_block_nodes, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"u8"}}})); break;
                }
                break;
            case TOKEN_U16:
                switch (current_parse_context)
                {
                    case PARSE_FUNCTION:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"u16"}}})); break;
                    case PARSE_FUNCTION_PARAMS:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.params,create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"u16"}}})); break;
                    case PARSE_BLOCK:
                        insert_ast_node(last_created_block_nodes, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"u16"}}})); break;
                }
            case TOKEN_U32:
                switch (current_parse_context)
                {
                    case PARSE_FUNCTION:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"u32"}}})); break;
                    case PARSE_FUNCTION_PARAMS:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.params,create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"u32"}}})); break;
                    case PARSE_BLOCK:
                        insert_ast_node(last_created_block_nodes, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"u32"}}})); break;
                }
            case TOKEN_S8:
                switch (current_parse_context)
                {
                    case PARSE_FUNCTION:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"s8"}}})); break;
                    case PARSE_FUNCTION_PARAMS:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.params,create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"s8"}}})); break;
                    case PARSE_BLOCK:
                        insert_ast_node(last_created_block_nodes, create_ast_node((ast_node_t){NODE_VARDECL, {.name_node = {"s8"}}})); break;
                }
            case TOKEN_S16:
                switch (current_parse_context)
                {
                    case PARSE_FUNCTION:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"s16"}}})); break;
                    case PARSE_FUNCTION_PARAMS:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.params,create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"s16"}}})); break;
                    case PARSE_BLOCK:
                        insert_ast_node(last_created_block_nodes, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"s16"}}})); break;
                }
            case TOKEN_S32:
                switch (current_parse_context)
                {
                    case PARSE_FUNCTION:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"s32"}}})); break;
                    case PARSE_FUNCTION_PARAMS:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.params, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"s32"}}})); break;
                    case PARSE_BLOCK:
                        insert_ast_node(last_created_block_nodes, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"s32"}}})); break;
                }
            case TOKEN_NAME:
                switch(current_parse_context)
                {
                    case PARSE_FUNCTION:
                        insert_ast_node(&last_created_fn_node->node_union.fn_node.name, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {token.value}}})); 
                        current_parse_context = PARSE_FUNCTION_PARAMS;                       
                        break;
                    case PARSE_BLOCK:
                }
            break;
        }
    }
}

