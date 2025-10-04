#include <stdbool.h>
#include "lexer.h"

static struct {
    const char *source;
} lexer_state;

static void lexer_push()
{
    
}

void lexer_init(const char *source)
{
    lexer_state.source = source;
}

bool lexer_next_token()
{

}