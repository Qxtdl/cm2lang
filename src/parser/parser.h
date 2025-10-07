#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"

typedef enum node_type {
    NODE_FUNCTION,
    NODE_VARDECL,
    NODE_BLOCK,
    NODE_EXPRESSION
} node_type_t;

struct ast_node;

typedef struct fn_node {
    const char *return_type;
    const char *name;
    const char **params;
    int param_count;
    struct ast_node **body;
} fn_node_t;

typedef struct vardecl_node {
    const char *type;
} vardecl_node_t;

typedef union {
    fn_node_t fn_node;
    vardecl_node_t vardecl_node;
} ast_node_union_t;

typedef struct ast_node {
    node_type_t type;
    ast_node_union_t node_union;
} ast_node_t;

void parser_process(void);

#endif // PARSER_H