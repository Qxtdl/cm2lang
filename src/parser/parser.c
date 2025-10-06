#include <stdio.h>
#include <stdbool.h>

#include "../globals.h"
#include "parser.h"
#include "../lexer/lexer.h"

static create_ast_node()
{
    ast_node_t *node = malloc(sizeof(ast_node_t));
    Check_alloc_fail(node, exit(1))
}

void parser_process(void)
{
    bool ir_continue = true;
    while (ir_continue)
    {
        token_t token = lexer_read_token(&ir_continue);
        switch (token.type)
        {
            case TOKEN_FN:
                create_ast_node();
                break;
        }
    }
}