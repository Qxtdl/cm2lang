#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

typedef enum {
    // braces and parantheses
    TOKEN_L_BRACE,
    TOKEN_R_BRACE,
    TOKEN_L_PARAN,
    TOKEN_R_PARAN,
    TOKEN_COMMENT,
    
    TOKEN_ASSIGN,
    TOKEN_SEMICOLON,
    
    TOKEN_CPU_SECTION,
    TOKEN_INSTRUCTIONS_SECTION,
    TOKEN_REGISTERS_SECTION,
    
    TOKEN_NAME,        // variable/function names
    TOKEN_NUMBER,      // numeric literals
    TOKEN_STRING,      // strings "example"
    
    TOKEN_EOF          // end of input
} token_type_t;

typedef struct {
    token_type_t type;
    const char *value;
} token_t;

void lexer_init(const char *source);
bool lexer_next_token(void);
token_t lexer_read_token(bool *parser_continue);

#endif // LEXER_H
