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
} vardecl_note_t;

typedef struct ast_node {
    node_type_t type;
    union {
        fn_node_t fn_node;
    };
} ast_node_t;

void parser_process(void);

#endif // PARSER_H