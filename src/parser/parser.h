#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>


typedef enum node_type {
    NODE_PROGRAM,
    NODE_NAME,
    NODE_FUNCTION,
    NODE_VARDECL,
    NODE_EXPRESSION,
    NODE_INLINE_ASM,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT
} node_type_t;

struct ast_node;

typedef struct {
    struct ast_node **ast_nodes;
    size_t num_children;
    size_t times_walked;
    size_t times_seen;
} ast_node_ptr_t;

typedef struct {
    ast_node_ptr_t body;
} program_node_t;

#define name_node_ptr ast_node_ptr_t
#define fn_node_ptr ast_node_ptr_t
#define vardecl_node_ptr ast_node_ptr_t
#define expr_node_ptr ast_node_ptr_t
#define inline_asm_node_ptr ast_node_ptr_t
#define if_statement_node_ptr ast_node_ptr_t

typedef struct {
    const char *value;
} name_node_t;

typedef struct {
    name_node_ptr return_type;
    name_node_ptr name;
    ast_node_ptr_t params;
    int param_count;
    ast_node_ptr_t body;
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
    ast_node_ptr_t body;
} if_statement_node_t;

typedef struct {
    ast_node_ptr_t condition;
    ast_node_ptr_t body;
} while_statement_node_t;

typedef union {
    program_node_t program_node;
    name_node_t name_node;
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

void parser_process(void);

ast_node_t *ast_walk(ast_node_ptr_t *ptr);
ast_node_t *ast_peek(ast_node_ptr_t *ptr, int offset);
ast_node_t *ast_see(ast_node_ptr_t *ptr);

#endif // PARSER_H