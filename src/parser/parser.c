#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../globals.h"
#include "parser.h"
#include "../lexer/lexer.h"

static ast_node_t *node = NULL; // last created one aswell

static ast_node_t *create_ast_node(token_type_t type, ast_node_union_t node_type)
{
    if (node == NULL)
        node = calloc(0, sizeof(ast_node_t));

    Check_alloc_fail(node, exit(1))
    switch (type)
    {
        case TOKEN_FN: node->type = NODE_FUNCTION; break;
    }
    memcpy(&node->node_union, &node_type, sizeof(ast_node_union_t));
    debug_printf("Created a new node\n");
    return node;
}

void edit_ast_node()
{
    
}

void parser_process(void)
{
    bool ir_continue = true;
    while (ir_continue)
    {
        token_t token = lexer_read_token(&ir_continue);
        switch (token.type)
        {
            case TOKEN_EOF:
                return;
            case TOKEN_FN:
                ast_node_t *fn_node = create_ast_node(
                    token.type, (ast_node_union_t){(fn_node_t){0}});
                break;
            case TOKEN_NORETURN:
                
        }
    }
}