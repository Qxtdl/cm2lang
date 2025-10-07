#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../globals.h"
#include "parser.h"
#include "../lexer/lexer.h"

static ast_node_t *current_node = NULL; // last created one aswell
static ast_node_t **ast_nodes = NULL;

static ast_node_t *create_ast_node(token_type_t type, ast_node_union_t node_type)
{
    static size_t num_ast_nodes = 0;

    if (current_node == NULL)
        current_node = calloc(1, sizeof(ast_node_t));
    if (ast_nodes == NULL)
    {
        ast_nodes = malloc(sizeof(ast_node_t *));

    }

    ast_nodes = realloc(ast_nodes, sizeof(ast_node_t *) * ++num_ast_nodes);
    switch (type)
    {
        case TOKEN_FN: current_node->type = NODE_FUNCTION; break;
    }
    memcpy(&current_node->node_union, &node_type, sizeof(ast_node_union_t));

    return current_node;
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
                break;
            case TOKEN_FN:
                ast_node_t *fn_node = create_ast_node(
                    token.type, (ast_node_union_t){(fn_node_t){0}});
                break;
            case TOKEN_MAIN_FUNCTION:
                    current_node->node_union.fn_node.name = "main"; break;
            case TOKEN_NORETURN:
                current_node->node_union.fn_node.return_type = "noreturn"; break;
            case TOKEN_NAME:
                switch (current_node->type)
                {
                    case NODE_FUNCTION:
                        current_node->node_union.fn_node.name = token.value; break;
                    case NODE_VARDECL:
                }
                break;
        }
    }
    debug_printf(current_node->node_union.fn_node.name);
    debug_printf("\n");
    debug_printf(current_node->node_union.fn_node.return_type);
    debug_printf("\n");
}
