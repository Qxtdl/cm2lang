#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>

typedef enum node_type {
    NODE_NULL,
    NODE_PROGRAM,
    NODE_NAME,
    NODE_BLOCK,
    NODE_FUNCTION,
    NODE_VARDECL,
    NODE_EXPRESSION,
    NODE_INLINE_ASM,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_BREAK
} node_type_t;

struct ast_node;
union ast_node_union;

typedef struct {
    struct ast_node **ast_nodes;
    size_t num_children;
    size_t times_walked;
    size_t times_seen;
} ast_node_ptr_t;

typedef struct {
    ast_node_ptr_t body;
    const char *pop_block_label;
} body_node_t;

typedef struct {
    body_node_t body_node;
} program_node_t;

#define name_node_ptr ast_node_ptr_t
#define fn_node_ptr ast_node_ptr_t
#define vardecl_node_ptr ast_node_ptr_t
#define expr_node_ptr ast_node_ptr_t
#define inline_asm_node_ptr ast_node_ptr_t
#define if_statement_node_ptr ast_node_ptr_t

/* TODO:
 * make the body a special struct for itsself with children bodies so that codegen
 * can know vars in scopes etc
*/

typedef struct {
    const char *value;
} name_node_t;

typedef struct {
    body_node_t body_node;
} block_node_t;

typedef struct {
    name_node_ptr return_type;
    name_node_ptr name;
    ast_node_ptr_t params;
    int param_count;
    body_node_t body_node;
} fn_node_t;    

typedef struct {
    bool is_preexisting;
    name_node_ptr type;
    name_node_ptr  name;
    name_node_ptr  init;
} vardecl_node_t;

typedef struct {
    ast_node_ptr_t ops; // Must contain exactly 2
    ast_node_ptr_t operator;
} expr_node_t;

typedef struct {
    ast_node_ptr_t inline_asm;
} inline_asm_node_t;

typedef struct {
    ast_node_ptr_t condition;
    body_node_t body_node;
} if_statement_node_t;

typedef struct {
    ast_node_ptr_t condition;
    body_node_t body_node;
} while_statement_node_t;

typedef union ast_node_union {
    program_node_t program_node;
    name_node_t name_node;
    block_node_t block_node;
    fn_node_t fn_node;
    vardecl_node_t vardecl_node;
    expr_node_t expr_node;
    inline_asm_node_t inline_asm_node;
    if_statement_node_t if_statement_node;
    while_statement_node_t while_statement_node;
} ast_node_union_t;

typedef struct ast_node {
    node_type_t type;
    ast_node_union_t node_union;
} ast_node_t;

typedef enum {
    PARSE_GLOBAL,           // 0
    PARSE_FUNCTION,         // 1
    PARSE_FUNCTION_PARAMS,  // 2
    PARSE_BLOCK,            // 3
    PARSE_VAR,              // 4
    PARSE_VAR_INIT,         // 5
    PARSE_INLINE_ASM,       // 6
    PARSE_IF_STATEMENT,     // 7
    PARSE_WHILE_STATEMENT   // 8
} parser_context_t;

void parser_process(void);

ast_node_t *ast_walk(ast_node_ptr_t *ptr);
ast_node_t *ast_peek(ast_node_ptr_t *ptr, int offset);
ast_node_t *ast_see(ast_node_ptr_t *ptr);

#endif // PARSER_H