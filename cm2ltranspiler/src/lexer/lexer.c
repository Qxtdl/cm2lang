#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "../globals.h"
#include "lexer.h"

const char *tokens[] = {
    "{",            // TOKEN_L_BRACE
    "}",            // TOKEN_R_BRACE
    "(",            // TOKEN_L_PAREN
    ")",            // TOKEN_R_PAREN
    "//",           // TOKEN_COMMENT

    "=",            // TOKEN_ASSIGN
    ";",            // TOKEN_SEMICOLON

    "cpu",          // TOKEN_CPU_SECTION
    "instructions", // TOKEN_INSTRUCTIONS_SECTION
    "registers",    // TOKEN_REGISTERS_SECTION

    "",             // TOKEN_NAME       (UNUSED)
    "",             // TOKEN_NUMBER     (UNUSED)
    "",             // TOKEN_STRING     (UNUSED)

    ""              // TOKEN_EOF        (UNUSED)
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

static void lexer_push_token(token_type_t type, char *token_value)
{
    lexer_state.current_token = realloc(lexer_state.current_token, lexer_state.tokens_size + sizeof(token_t));
    lexer_state.current_token[lexer_state.tokens_size / sizeof(token_t)].type = type;
    lexer_state.current_token[lexer_state.tokens_size / sizeof(token_t)].value = token_value;
    lexer_state.tokens_size += sizeof(token_t);
    debug_printf("%d %s\n", type, token_value);
}

static char *token_dup(size_t token_len, const char *token)
{
    char *new_token = malloc(token_len + 1);
    new_token[token_len] = '\0';
    strncpy(new_token, token, token_len);
    return new_token;
}

#define lexer_advance_till_non_whitespace \
while (*lexer_state.current_pos != '\0' && isspace(*lexer_state.current_pos)) \
    lexer_state.current_pos++;

#define lexer_advance_till_whitespace \
while (*lexer_state.current_pos != '\0' && !isspace(*lexer_state.current_pos) && !ispunct(*lexer_state.current_pos)) \
    lexer_state.current_pos++;

#define lexer_advance_till_whitespace_or_semicolon \
while (*lexer_state.current_pos != '\0' && !isspace(*lexer_state.current_pos) &&  !ispunct(*lexer_state.current_pos) && *lexer_state.current_pos != ';') \
    lexer_state.current_pos++;

#define lexer_advance_till_semicolon \
while (*lexer_state.current_pos != '\0' && *lexer_state.current_pos != ';') \
    lexer_state.current_pos++;

#define lexer_advance_till_quotes \
while (*lexer_state.current_pos != '\0' && *lexer_state.current_pos != '\"') \
    lexer_state.current_pos++;

[[nodiscard]]
bool lexer_next_token(void)
{
    lexer_advance_till_non_whitespace
    if (*lexer_state.current_pos == '\0') {
        lexer_push_token(TOKEN_EOF, NULL);
        return false; // no more tokens
    }
    for (size_t i = 0; i < sizeof(tokens) / sizeof(tokens[0]); i++) {
        size_t token_length = strlen(tokens[i]);
        if (token_length > 0 && strncmp(lexer_state.current_pos, tokens[i], token_length) == 0)
        {   
            lexer_push_token(i, token_dup(token_length, lexer_state.current_pos));
            lexer_state.current_pos += token_length;
            return true;
        }
    }
    if (*lexer_state.current_pos == '\"') {
        const char *string_start = ++lexer_state.current_pos;
        lexer_advance_till_quotes
        const char *string_end = lexer_state.current_pos;
        size_t string_length = string_end - string_start;
        lexer_push_token(TOKEN_STRING, token_dup(string_length, string_start));
        lexer_state.current_pos++;  // Because we reached the closing ", we increment by 1 character to continue lexing.
        return true;
    }
    if (isdigit(*lexer_state.current_pos)) {
        const char *digit_start = lexer_state.current_pos;
        lexer_advance_till_whitespace_or_semicolon
        const char *digit_end = lexer_state.current_pos;
        size_t digit_length = digit_end - digit_start;
        lexer_push_token(TOKEN_NUMBER, token_dup(digit_length, digit_start));
    }
    else {
        const char *name_start = lexer_state.current_pos;
        lexer_advance_till_whitespace_or_semicolon
        const char *name_end = lexer_state.current_pos;
        size_t name_len = name_end - name_start;
        lexer_push_token(TOKEN_NAME, token_dup(name_len, name_start));   
    }
    
    return true;
}

token_t lexer_read_token(bool *ir_continue)
{
    static token_t *current_token = NULL;
    static size_t times_read = 0;
    if (current_token == NULL)
        current_token = lexer_state.current_token;
    if (++times_read >= lexer_state.tokens_size / sizeof(token_t))
        *ir_continue = false;
    else
        *ir_continue = true;
    return *current_token++;
}
