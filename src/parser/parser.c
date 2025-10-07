#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../globals.h"
#include "parser.h"
#include "../lexer/lexer.h"

typedef union {
    fn_node_t fn_node
} ast_node_type_t;

static ast_node_t *create_ast_node(token_type_t type, ast_node_type_t node_type)
{
    static ast_node_t *node = NULL;
    if (node == NULL)
        node = calloc(0, sizeof(ast_node_t));

    Check_alloc_fail(node, exit(1))
    switch (type)
    {
        case TOKEN_FN: node->type = NODE_FUNCTION; break;
    }
    debug_printf("Created a new node\n");
    return node;
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
                ast_node_t *fn_node = create_ast_node(token.type);
                break;
        }
    }
}