#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer/lexer.h"

int main(int argc, char* argv[])
{
    if (argc >= 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h")) == 0)
        printf("Usage: cm2lc <sourcefile>\n");
    
    FILE *source_file = fopen(argv[1], "rb");
    if (source_file == NULL)
    {
        perror("Error opening file");
        return -1;
    }

    fseek(source_file, 0, SEEK_END);
    long file_size = ftell(source_file);
    rewind(source_file);

    char *source_code = (char *)malloc(file_size + 1);
    if (source_code == NULL)
    {
        perror("Memory allocation failed");
        return -1;
    }

    size_t read_size = fread(source_code, 1, file_size, source_file);
    if ((long)read_size != file_size)
    {
        perror("Error reading source file");
        free(source_code);
        return -1;
    }
    source_code[file_size] = '\0';
    fclose(source_file);

    printf("Source Code:\n%s\n", source_code);
}