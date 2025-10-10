#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"

typedef enum node_type {
    NODE_PROGRAM,
    NODE_NAME,
    NODE_FUNCTION,
    NODE_VARDECL,
    NODE_BLOCK,
    NODE_EXPRESSION
} node_type_t;

struct ast_node;

typedef struct {
    struct ast_node **ast_nodes;
    size_t num_children;
    size_t times_walked;
} ast_node_ptr_t;

typedef struct {
    ast_node_ptr_t body;
} program_node_t;

typedef struct {
    const char *value;
} name_node_t;

typedef struct {
    ast_node_ptr_t return_type;
    ast_node_ptr_t name;
    ast_node_ptr_t params;
    int param_count;
    ast_node_ptr_t body;
} fn_node_t;    

typedef struct {
    ast_node_ptr_t type;
    ast_node_ptr_t name;
    ast_node_ptr_t init;
} vardecl_node_t;

typedef union {
    program_node_t program_node;
    name_node_t name_node;
    fn_node_t fn_node;
    vardecl_node_t vardecl_node;
} ast_node_union_t;

typedef struct ast_node {
    node_type_t type;
    ast_node_union_t node_union;
} ast_node_t;

void parser_process(void);
ast_node_t *ast_walk(ast_node_ptr_t *ptr);

#endif // PARSER_H