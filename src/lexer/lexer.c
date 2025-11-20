#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "../globals.h"
#include "lexer.h"

const char *const tokens[] = {
    [TOKEN_L_BRACE]     = "{",        // TOKEN_L_BRACE
    [TOKEN_R_BRACE]     = "}",        // TOKEN_R_BRACE
    [TOKEN_L_PARAN]     = "(",        // TOKEN_L_PAREN
    [TOKEN_R_PARAN]     = ")",        // TOKEN_R_PAREN
    [TOKEN_COMMENT]     = "//",       // TOKEN_COMMENT

    [TOKEN_V16]         = "v16",      // TOKEN_V16
    [TOKEN_V32]         = "v32",      // TOKEN_V32
    [TOKEN_VOID]        = "void",     // TOKEN_VOID

    [TOKEN_FN]          = "fn",       // TOKEN_FN
    [TOKEN_NORETURN]    = "noreturn", // TOKEN_NORETURN
    [TOKEN_NOPARAM]     = "noparam",  // TOKEN_NORETURN

    [TOKEN_IF]          = "if",       // TOKEN_IF
    [TOKEN_WHILE]       = "while",    // TOKEN_WHILE
    [TOKEN_BREAK]       = "break",    // TOKEN_BREAK

    [TOKEN_ASM]         = "asm",      // TOKEN_ASM

    [TOKEN_ALWAYS]      = "always",   // TOKEN_ALWAYS

    [TOKEN_ASSIGN]      = "=",        // TOKEN_ASSIGN
    [TOKEN_EQUALS]      = "==",       // TOKEN_EQUALS
    [TOKEN_NOT_EQUALS]  = "!=",       // TOKEN_NOT_EQUALS
    [TOKEN_SEMICOLON]   = ";",        // TOKEN_SEMICOLON

    [TOKEN_PLUS]        = "+",        // TOKEN_PLUS
    [TOKEN_MINUS]       = "-",        // TOKEN_MINUS
    [TOKEN_BITWISE_AND] = "&",        // TOKEN_BITWISE_AND
    [TOKEN_BITWISE_OR]  = "|",        // TOKEN_BITWISE_OR
    [TOKEN_BITWISE_XOR] = "^",        // TOKEN_BITWISE_XOR

    "",                               // TOKEN_NAME       (UNUSED)
    "",                               // TOKEN_NUMBER     (UNUSED)
    "",                               // TOKEN_STRING     (UNUSED)

    "",                               // TOKEN_EOF        (UNUSED)
    [TOKEN_NULL] = " "
};

static struct {
    const char *source;
    const char *current_pos; // current position in the source code
    const char *current_newline;
    token_t *current_token;
    size_t tokens_size; // number of tokens allocated in bytes
} lexer_state;

void lexer_init(const char *source)
{
    lexer_state.source = source;
    lexer_state.current_token = NULL;
    lexer_state.current_pos = source;
    lexer_state.current_newline = source;
    lexer_state.tokens_size = 0;
}

static size_t line = 0;

static void lexer_push_token(token_type_t type, const char *token_value)
{
    lexer_state.current_token = realloc(lexer_state.current_token, lexer_state.tokens_size + sizeof(token_t));
    lexer_state.current_token[lexer_state.tokens_size / sizeof(token_t)].type = type;
    lexer_state.current_token[lexer_state.tokens_size / sizeof(token_t)].value = token_value;
    lexer_state.current_token[lexer_state.tokens_size / sizeof(token_t)].line = line;
    lexer_state.tokens_size += sizeof(token_t);
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

#define lexer_advance_till_newline \
while (*lexer_state.current_pos != '\0' && *lexer_state.current_pos != '\n') \
    lexer_state.current_pos++; 

[[nodiscard]]
bool lexer_next_token(void)
{
    if ((lexer_state.current_newline = strstr(lexer_state.current_newline, "\n"))) line++;
    lexer_advance_till_non_whitespace
    if (*lexer_state.current_pos == '\0') {
        lexer_push_token(TOKEN_EOF, NULL);
        return false; // no more tokens
    }
    token_type_t match_id;
    const char *match = NULL;
    for (size_t i = 0; i < TOKEN_LAST_USABLE_TOKEN; i++) {
        if (!strncmp(lexer_state.current_pos, tokens[i], strlen(tokens[i]))) {
            match_id = i;
            match = tokens[i];
        }
        if (i == TOKEN_LAST_USABLE_TOKEN - 1 && match) {
            size_t match_len = strlen(match);
            lexer_push_token(match_id, token_dup(match_len, lexer_state.current_pos));
            if (match_id == TOKEN_COMMENT)
                lexer_advance_till_newline
            else
                lexer_state.current_pos += match_len;
            return true;
        }
    }
    if (*lexer_state.current_pos == '\"') {
        // TODO: Add escape support (like \")
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

static token_t *current_token = NULL;
static size_t times_read = 0;

token_t lexer_read_token(bool *parser_continue)
{
    if (current_token == NULL)
        current_token = lexer_state.current_token;
    if (++times_read >= lexer_state.tokens_size / sizeof(token_t))
        *parser_continue = false;
    else
        *parser_continue = true;
    return *current_token++;
}

token_t lexer_peek_token(size_t offset)
{
    return current_token[times_read - 1 + offset];
}
