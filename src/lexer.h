#ifndef LEXER_H
#define LEXER_H

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
} token_t;

token_t* tokenize(const char **input);
char* token_to_string(const char **input);


#endif // LEXER_H