#include "../globals.h"
#include "syntaxanalyzer.h"
#include "parser.h"
#include "../cflags.h"

extern parser_context_t current_context;

#define syntax_analyzer_abort(reasson, ...) app_abort("Syntax Analyzer", "Syntax Analyzer detected a issue in your code: " reasson, __VA_ARGS__)

void syntax_analyze(token_t token)
{
   if (is_cflag_enabled(cflags[FLAG_NO_SYNTAX_ANALYZER]))
      return;
   
   // Check for missing semicolon
   if (current_context == PARSE_VAR_INIT && lexer_peek_token(1).type == TOKEN_NUMBER && lexer_peek_token(2).type != TOKEN_SEMICOLON) {
      syntax_analyzer_abort("Expected semicolon, instead got %s at line %d", lexer_peek_token(1).value, lexer_peek_token(1).line)
   }
}