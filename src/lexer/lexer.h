#ifndef LEXER_H
#define LEXER_H

#include "stdbool.h"

typedef enum {
    // entry point for all cm2l programs
    TOKEN_MAIN_FUNCTION,

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
    TOKEN_NORETURN,
    TOKEN_NOPARAM,

    TOKEN_ASSIGN,
    TOKEN_SEMICOLON,
    
    TOKEN_PLUS,
    TOKEN_MINUS,

    TOKEN_IDENTIFIER,  // variable/function names
    TOKEN_NUMBER,      // numeric literals
    
    TOKEN_EOF          // end of input
} token_type_t;

static const char *tokens[] =
{
    "main",     // TOKEN_MAIN_FUNCTION

    "(",        // TOKEN_L_BRACE
    ")",        // TOKEN_R_BRACE
    "{",        // TOKEN_L_PARAN
    "}",        // TOKEN_R_PARAN

    "u8",       // TOKEN_U8
    "u16",      // TOKEN_U16
    "u32",      // TOKEN_U32
    "s8",       // TOKEN_S8
    "s16",      // TOKEN_S16
    "s32",      // TOKEN_S32
    "void",     // TOKEN_VOID
    "noreturn", // TOKEN_NORETURN
    "noparam",  // TOKEN_NORETURN

    "=",        // TOKEN_ASSIGN
    ";",        // TOKEN_SEMICOLON
    "+",        // TOKEN_PLUS
    "-",        // TOKEN_MINUS

    "",         // TOKEN_IDENTIFIER (UNUSED)
    "",         // TOKEN_NUMBER     (UNUSED)

    ""          // TOKEN_EOF        (UNUSED)
};

typedef struct {
    token_type_t type;
    const char *value;
    size_t length;
} token_t;

void lexer_init();
bool lexer_next_token();

#endif // LEXER_H
