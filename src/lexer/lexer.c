#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "../globals.h"
#include "lexer.h"

const char *tokens[] = {
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

    "fn",       // TOKEN_FN
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

static struct {
    const char *source;
    const char *current_pos; // current position in the source code
    token_t *current_token;
    size_t tokens_size; // number of tokens allocated in bytes
} lexer_state;

void lexer_init(const char *source)
{
    lexer_state.source = source;
    lexer_state.current_token = NULL;
    lexer_state.current_pos = source;
    lexer_state.tokens_size = 0;
}

static void lexer_push_token(token_type_t type, const char *token_value)
{
    if (lexer_state.current_token == NULL)
    lexer_state.current_token = (token_t *)malloc(sizeof(token_t));
    else
    lexer_state.current_token = (token_t *)realloc(lexer_state.current_token, lexer_state.tokens_size + sizeof(token_t));
    Check_alloc_fail(lexer_state.current_token, exit(1))
    lexer_state.current_token[lexer_state.tokens_size / sizeof(token_t)].type = type;
    lexer_state.current_token[lexer_state.tokens_size / sizeof(token_t)].value = token_value;
    
    if (type == TOKEN_EOF)
        debug_printf("EOF\n");
    if (lexer_state.current_token[lexer_state.tokens_size / sizeof(token_t)].value)
        debug_printf("%s\n", lexer_state.current_token[lexer_state.tokens_size / sizeof(token_t)].value);

    lexer_state.tokens_size += sizeof(token_t);
}   


#define lexer_advance_till_non_whitespace \
while (isspace(*lexer_state.current_pos)) \
    lexer_state.current_pos++;

#define lexer_advance_till_whitespace \
while (!isspace(*lexer_state.current_pos)) \
    lexer_state.current_pos++;

#define lexer_advance_till_semicolon \
while (*lexer_state.current_pos != ';') \
    lexer_state.current_pos++;

static char *token_dup(size_t token_len, const char *token)
{
    char *new_token = malloc(token_len + 1);
    Check_alloc_fail(new_token, exit(1))
    new_token[token_len] = '\0';
    strncpy(new_token, token, token_len);
    return new_token;
}

bool lexer_next_token(void)
{
    lexer_advance_till_non_whitespace
    
    if (*lexer_state.current_pos == '\0') 
    {
        lexer_push_token(TOKEN_EOF, NULL);
        return false; // no more tokens
    }
    
    for (size_t i = 0; i < sizeof(tokens) / sizeof(tokens[0]); i++)
    {
        size_t token_length = strlen(tokens[i]);
        if (token_length > 0 && strncmp(lexer_state.current_pos, tokens[i], token_length) == 0)
        {
            // char *new_token_name = malloc(token_length + 1);
            // Check_alloc_fail(new_token_name, exit(1))
            // new_token_name[token_length + 1] = '\0';
            // strncpy(new_token_name, lexer_state.current_pos, token_length);
            
            lexer_push_token(i, token_dup(token_length, lexer_state.current_pos));
            lexer_state.current_pos += token_length;
            
            if (i == TOKEN_U8 || i == TOKEN_U32 || i == TOKEN_S8 || i == TOKEN_S16 || i == TOKEN_S32)
            {
                lexer_advance_till_non_whitespace
                const char *var_start = lexer_state.current_pos;
                lexer_advance_till_whitespace
                size_t var_length = lexer_state.current_pos - var_start;
                lexer_push_token(TOKEN_IDENTIFIER, token_dup(var_length, var_start));
            }

            if (i == TOKEN_ASSIGN)
            {
                lexer_advance_till_non_whitespace
                const char *number_start = lexer_state.current_pos;
                lexer_advance_till_semicolon
                size_t number_length = lexer_state.current_pos - number_start;
                lexer_push_token(TOKEN_NUMBER, token_dup(number_length, number_start));
            }

            return true;
        }
    }

    perror("lexer.c: Some invalid tokens were detected. Is your code written correctly?\n");
    exit(1);
}

static token_t *current_token = NULL;
static size_t times_read = 0;

token_t lexer_read_token(bool *ir_continue)
{
    if (current_token == NULL)
        current_token = lexer_state.current_token;

    if (times_read++ >= lexer_state.tokens_size / sizeof(token_t))
        *ir_continue = false;
    else
        *ir_continue = true;
    
    return *current_token++;
}

void lexer_advance_token()
{
    times_read++;
    current_token++;
}