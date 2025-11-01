#include <stdlib.h>
#include <string.h>

#include "../globals.h"
#include "parser.h"
#include "../lexer/lexer.h"

ast_node_t *program_node = NULL;
ast_node_t *cpu_section_node = NULL;
ast_node_t *instructions_section_node = NULL;
ast_node_t *registers_section_node = NULL;
ast_node_t *last_created_section_node = NULL;
ast_node_t *last_created_node = NULL;
ast_node_ptr_t *last_created_block_nodes = NULL;

static ast_node_t *create_ast_node(bool replace_last_created_node, ast_node_t initalizer_node)
{
    ast_node_t *node = malloc(sizeof(ast_node_t));
    memcpy(node, &initalizer_node, sizeof(ast_node_t));
    if (replace_last_created_node)
        last_created_node = node;
    switch (initalizer_node.type) {
    case NODE_PROGRAM:
        program_node = node; break;
    case NODE_SECTION:
        last_created_section_node = node; break;
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
    if (!ptr) return ptr;
    if (ptr->times_walked < ptr->num_children)
        return ptr->ast_nodes[ptr->times_walked++];
    return NULL;
}

ast_node_t *ast_see(ast_node_ptr_t *ptr)
{
    if (!ptr) return ptr;
    if (ptr->times_seen < ptr->num_children)
        return ptr->ast_nodes[ptr->times_seen++];
    ptr->times_seen = 0;
    return NULL;
}

ast_node_t *ast_peek(ast_node_ptr_t *ptr, int offset)
{
    if (!ptr) return ptr;
    if (ptr->times_walked + offset - 1 < ptr->num_children)
        return ptr->ast_nodes[ptr->times_walked + offset - 1];
    return NULL;
}

typedef enum {
    PARSE_GLOBAL,
    PARSE_CPU_SECTION_NAME,
    PARSE_INSTRUCTIONS_SECTION,
    PARSE_REGISTERS_SECTION,
    PARSE_ASMDECL // this one is also reused for the registers section
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
    bool parser_continue = true;
    
    push_context(PARSE_GLOBAL);
    create_ast_node(true, (ast_node_t){NODE_PROGRAM});
    while (parser_continue) 
    {
        token_t token = lexer_read_token(&parser_continue);
        switch (token.type) {
        case TOKEN_CPU_SECTION:
            insert_ast_node(&program_node->node_union.program_node.body, 
                (cpu_section_node = create_ast_node(true, (ast_node_t){NODE_SECTION, {{{0}}}})));
            insert_ast_node(&last_created_section_node->node_union.section_node.type, 
                create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {"cpu"}}}));
            push_context(PARSE_CPU_SECTION_NAME);
            break;
        case TOKEN_INSTRUCTIONS_SECTION:
            insert_ast_node(&cpu_section_node->node_union.section_node.body, 
                (instructions_section_node = create_ast_node(true, (ast_node_t){NODE_SECTION, {{{0}}}})));
            insert_ast_node(&last_created_section_node->node_union.section_node.type, 
                create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {"instructions"}}}));
            push_context(PARSE_INSTRUCTIONS_SECTION);
            break;
        case TOKEN_REGISTERS_SECTION:
            insert_ast_node(&cpu_section_node->node_union.section_node.body,
                (registers_section_node = create_ast_node(true, (ast_node_t){NODE_SECTION, {{{0}}}})));
            insert_ast_node(&last_created_section_node->node_union.section_node.type, 
                create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {"registers"}}}));
            push_context(PARSE_REGISTERS_SECTION);
            break;
        case TOKEN_STRING:
            switch (current_context) {
            case PARSE_CPU_SECTION_NAME:
                insert_ast_node(&last_created_section_node->node_union.section_node.name, 
                    create_ast_node(true, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
                pop_context();
                break;
            case PARSE_ASMDECL:
                insert_ast_node(&last_created_node->node_union.asmdecl_node.value, 
                    create_ast_node(false, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
                pop_context();
                break;
            }
            break;
        case TOKEN_NAME:
            switch (current_context) {
            case PARSE_INSTRUCTIONS_SECTION:
                insert_ast_node(&instructions_section_node->node_union.section_node.body, 
                    create_ast_node(true, (ast_node_t){NODE_ASMDECL, {{{0}}}}));
                insert_ast_node(&last_created_node->node_union.asmdecl_node.name, 
                    create_ast_node(false, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
                push_context(PARSE_ASMDECL);
                break;
            case PARSE_REGISTERS_SECTION:
                insert_ast_node(&registers_section_node->node_union.section_node.body, 
                    create_ast_node(true, (ast_node_t){NODE_ASMDECL, {{{0}}}}));
                insert_ast_node(&last_created_node->node_union.asmdecl_node.name, 
                    create_ast_node(false, (ast_node_t){NODE_NAME, {.name_node = {token.value}}}));
                push_context(PARSE_ASMDECL);
            }
            break;
        case TOKEN_SEMICOLON:
            // MODO: Add switch
            pop_context(); break;
        }
    }
}