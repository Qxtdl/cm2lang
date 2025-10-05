#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "lexer/lexer.h"
#include "ir/ir.h"

extern struct ir_state_s ir_state;

int main(int argc, char* argv[])
{
    if (argc >= 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h")) == 0)
    {
        printf("Usage: cm2lc <sourcefile>\n");
        return 0;
    }
    
    FILE *source_file = fopen(argv[1], "rb");
    if (source_file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    fseek(source_file, 0, SEEK_END);
    long file_size = ftell(source_file);
    rewind(source_file);

    char *source_code = (char *)malloc(file_size + 1);
    Check_alloc_fail(source_code, exit(-1))

    size_t read_size = fread(source_code, 1, file_size, source_file);
    if ((long)read_size != file_size)
    {
        perror("Error reading source file");
        free(source_code);
        return 1;
    }
    source_code[file_size] = '\0';
    fclose(source_file);

    lexer_init(source_code);
    while (lexer_next_token());
    debug_printf("main.c: Finished lexing\n");
    ir_init();
    debug_printf("main.c: Finished ir_init\n");
    ir_process();
    debug_printf("main.c: Finished ir processing\n");

    FILE *compiled = fopen(argv[2], "w");
    if (compiled == NULL)
    {
        perror("Error writing compiled to file");
        return 1;
    }
    fwrite(ir_state.compiled, 1, ir_state.compiled_len, compiled);
    fclose(compiled);
}