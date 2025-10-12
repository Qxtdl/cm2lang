#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

typedef enum {
    // braces and parantheses
    TOKEN_L_PARAN,
    TOKEN_R_PARAN,
    TOKEN_L_BRACE,
    TOKEN_R_BRACE,

    // data types, also a noreturn for syntactic sugar
    TOKEN_U8,
    TOKEN_U16,
    TOKEN_U32,
    TOKEN_S8,
    TOKEN_S16,
    TOKEN_S32,
    TOKEN_VOID,
    
    TOKEN_FN,
    TOKEN_NORETURN,
    TOKEN_NOPARAM,

    TOKEN_ASSIGN,
    TOKEN_SEMICOLON,
    
    TOKEN_PLUS,
    TOKEN_MINUS,

    TOKEN_NAME,        // variable/function names
    TOKEN_NUMBER,      // numeric literals
    
    TOKEN_EOF          // end of input
} token_type_t;

typedef struct {
    token_type_t type;
    char *value;
} token_t;

void lexer_init(const char *source);
bool lexer_next_token();
token_t lexer_read_token(bool *ir_continue);
void lexer_advance_token();

#endif // LEXER_H
