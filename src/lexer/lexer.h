#ifndef LEXER_H
#define LEXER_H
#include <stdbool.h>

typedef enum {
    L_PARAN,
    R_PARAN,
    L_BRACE,
    R_BRACE,
    TOKEN_ASSIGN,      // =
    TOKEN_EQUALS,      // ==
    TOKEN_PLUS,        // +
    TOKEN_MINUS,       // -
    TOKEN_STAR,        // *
    TOKEN_SLASH,       // /
    TOKEN_SEMICOLON,   // ;
    TOKEN_COMMA,       // ,
    TOKEN_IDENTIFIER,  // variable/function names
    TOKEN_NUMBER,      // numeric literals
    TOKEN_EOF          // end of input
} token_type_t;

typedef struct {
    token_type_t type;
    char *value;
} token_t;

void lexer_init();
bool lexer_next_token();

#endif // LEXER_H