#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "../globals.h"
#include "lexer.h"

static struct {
    const char *source;
    const char *current_pos; // current position in the source code
    token_t *current_token;
    size_t tokens_size; // number of tokens allocated in bytes
} lexer_state;

static void lexer_push_token(token_type_t type, const char *start, size_t length)
{
    if (lexer_state.current_token == NULL)
        lexer_state.current_token = (token_t *)malloc(sizeof(token_t));
    else
        lexer_state.current_token = (token_t *)realloc(lexer_state.current_token, lexer_state.tokens_size + sizeof(token_t));
    lexer_state.current_token[lexer_state.tokens_size / sizeof(token_t)].type = type;
    lexer_state.current_token[lexer_state.tokens_size / sizeof(token_t)].value = start;
    lexer_state.current_token[lexer_state.tokens_size / sizeof(token_t)].length = length;
    lexer_state.tokens_size += sizeof(token_t);
    if (type == TOKEN_EOF)
        debug_printf("EOF\n");
    debug_printf("%.*s\n", (int)length, start);
}   

void lexer_init(const char *source)
{
    lexer_state.source = source;
    lexer_state.current_token = NULL;
    lexer_state.current_pos = source;
    lexer_state.tokens_size = 0;
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

bool lexer_next_token()
{
    lexer_advance_till_non_whitespace

    if (*lexer_state.current_pos == '\0') 
    {
        lexer_push_token(TOKEN_EOF, NULL, 0);
        return false; // no more tokens
    }
    
    for (size_t i = 0; i < sizeof(tokens) / sizeof(tokens[0]); i++)
    {
        size_t token_length = strlen(tokens[i]);
        if (token_length > 0 && strncmp(lexer_state.current_pos, tokens[i], token_length) == 0)
        {
            lexer_push_token(i, lexer_state.current_pos, token_length);
            lexer_state.current_pos += token_length;

            // Case that its a var
            if (i == TOKEN_U8 || i == TOKEN_U32 || i == TOKEN_S8 || i == TOKEN_S16 || i == TOKEN_S32)
            {
                lexer_advance_till_non_whitespace
                const char *var_start = lexer_state.current_pos;
                lexer_advance_till_whitespace
                size_t var_length = lexer_state.current_pos - var_start;
                lexer_push_token(TOKEN_IDENTIFIER, var_start, var_length);
            }

            if (i == TOKEN_ASSIGN)
            {
                lexer_advance_till_non_whitespace
                const char *number_start = lexer_state.current_pos;
                lexer_advance_till_semicolon
                size_t number_length = lexer_state.current_pos - number_start;
                lexer_push_token(TOKEN_NUMBER, number_start, number_length);
            }

            return true;
        }
    }

    printf("lexer.c: Some tokens were not detected. Is your code written correctly?\n");
    return false;   // if we get here, it means we somehow went pass the EOF and the tokens
}