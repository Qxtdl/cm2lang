#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>

typedef enum node_type {
    NODE_PROGRAM,
    NODE_NAME,
    NODE_SECTION,
    NODE_ASMDECL
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

typedef struct {
    const char *value;
} name_node_t;

typedef struct {
    ast_node_ptr_t body;
    ast_node_ptr_t type;
    ast_node_ptr_t name;
} section_node_t;

typedef struct {
    ast_node_ptr_t name;
    ast_node_ptr_t value;
} asmdecl_node_t;

typedef union {
    program_node_t program_node;
    name_node_t name_node;
    section_node_t section_node;
    asmdecl_node_t asmdecl_node;
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