#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "transpiler/transpiler.h"

int g_argc;
char **g_argv;

void compiler_warn(const char *message)
{
    printf("[WARNING]: %s\n", message);
}

static const char *read_file(const char *filepath)
{
   FILE *file = fopen(filepath, "rb");
   if (file == NULL) {
      app_abort("read_file()", "Error opening file %s!", filepath)
   }

   fseek(file, 0, SEEK_END);
   long file_size = ftell(file);
   rewind(file);

   char *file_contents = malloc(file_size + 1);

   size_t read_size = fread(file_contents, 1, file_size, file);
   if ((long)read_size != file_size) {
      app_abort("read_file()", "Error reading contents of file %s!", filepath)
   }

   file_contents[file_size] = '\0';
   fclose(file);

   return file_contents;
}

static void write_file(const char *filepath, const char *text)
{
   FILE *file = fopen(filepath, "wb");
   if (file == NULL) {
      app_abort("write_file()", "Error opening file %s!", filepath)
   }

   fwrite(text, 1, strlen(text), file);
}

extern struct transpiler_state transpiler_state;

int main(int argc, char* argv[])
{
   g_argc = argc;
   g_argv = argv;

   if (argc >= 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h")) == 0) {
      printf("Usage: cm2lt <asm file>\n");
      return 0;
   }

   const char *hd_code = read_file(argv[1]);
   const char *asm_code = read_file(argv[2]);

   lexer_init(hd_code);
   while (lexer_next_token());
   parser_process();
   transpiler_init(asm_code);
   transpiler_process();

   FILE *transpiled = fopen(argv[3], "wb");

   fwrite(transpiler_state.transpiled, 1, transpiler_state.transpiled_len, transpiled);
   fclose(transpiled);

   return 0;
}
