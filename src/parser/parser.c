#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../globals.h"
#include "parser.h"
#include "../lexer/lexer.h"

static ast_node_t *last_created_node = NULL;

static ast_node_t *create_ast_node(ast_node_t initalizer_node)
{
    ast_node_t *node = calloc(1, sizeof(ast_node_t));
    memcpy(node, &initalizer_node, sizeof(ast_node_t));
    last_created_node = node;
    return node;
}

typedef struct {
    ast_node_t **children;
    size_t num_children;
} children_ptr_t;

static void insert_ast_node(struct ast_node ***children, ast_node_t *new_ast_node)
{
    static children_ptr_t *known_children_ptrs = NULL;
    static size_t known_children_ptrs_size = 0;
    if (known_children_ptrs == NULL)
        known_children_ptrs = malloc(sizeof(children_ptr_t)); 

    for (size_t i = 0; i < known_children_ptrs_size; i++)
    {
        if (known_children_ptrs[i].children == *children)
        {
            // It is a known pointer
            known_children_ptrs[i].children = realloc(known_children_ptrs[i].children, sizeof(struct ast_node *) * (++(known_children_ptrs[i].num_children)));
            known_children_ptrs[i].children[known_children_ptrs[i].num_children - 1] = new_ast_node;
            return;
        }
    }
    known_children_ptrs = realloc(known_children_ptrs, sizeof(children_ptr_t) * (++known_children_ptrs_size));
    known_children_ptrs[known_children_ptrs_size - 1].children = malloc(sizeof(struct ast_node *));
    *children = known_children_ptrs[known_children_ptrs_size - 1].children;
    known_children_ptrs[known_children_ptrs_size - 1].num_children = 1;
    known_children_ptrs[known_children_ptrs_size - 1].children[known_children_ptrs[known_children_ptrs_size - 1].num_children++] = new_ast_node;
}

static ast_node_t *last_created_fn_node = NULL;

void parser_process(void)
{           
    ast_node_t *program = create_ast_node((ast_node_t){NODE_PROGRAM, {.program_node = {0}}});
    bool ir_continue = true;
    while (ir_continue)
    {
        token_t token = lexer_read_token(&ir_continue);
        if (!ir_continue) break; // TOKEN_EOF
        switch (token.type)
        {
            case TOKEN_FN:
                insert_ast_node(&program->node_union.program_node.body, (last_created_fn_node = &(ast_node_t){NODE_FUNCTION, {.fn_node = {0}}})); break;
            case TOKEN_NORETURN:
                insert_ast_node(&last_created_fn_node->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"noreturn"}}})); break;
            case TOKEN_NOPARAM:
                insert_ast_node(&last_created_fn_node->node_union.fn_node.params, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"noparam"}}})); break;
            case TOKEN_U8:
                insert_ast_node(&program->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"u8"}}})); break;
            case TOKEN_U16:
                insert_ast_node(&program->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"u16"}}})); break;
            case TOKEN_U32:
                insert_ast_node(&program->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"u32"}}})); break;
            case TOKEN_S8:
                insert_ast_node(&program->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"s8"}}})); break;
            case TOKEN_S16:
                insert_ast_node(&program->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"s16"}}})); break;
            case TOKEN_S32:
                insert_ast_node(&program->node_union.fn_node.return_type, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {"s32"}}})); break;
            case TOKEN_NAME:
                switch(last_created_node->type)
                {
                    case NODE_FUNCTION:
                        insert_ast_node(&last_created_node->node_union.fn_node.name, create_ast_node((ast_node_t){NODE_NAME, {.name_node = {token.value}}})); break;
                    case NODE_VARDECL:
                        insert_ast_node(&last_created_node->node_union.vardecl_node.name, create_ast_node((ast_node_t){NODE_VARDECL, {.vardecl_node = {0}}})); break;
                }
            break;
        }
    }
    debug_printf(last_created_fn_node->node_union.fn_node.return_type[0]);
}
