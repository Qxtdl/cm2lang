#ifndef CFLAGS_H
#define CFLAGS_H

#include <stdbool.h>

enum {
   FLAG_VERBOSE_ASM,
   FLAG_STACK_INIT,
   FLAG_RAM_MASK,
   FLAG_ASM_COMMENT,
   FLAG_HALF_SIZE,
   FLAG_NO_INIT_ZERO_REG,
   FLAG_NO_SYNTAX_ANALYZER,

   // ISA extensions
   FLAG_ZJREG_EXTENSION
};

static const char *const cflags[] = {
   [FLAG_VERBOSE_ASM]         = "-fverbose-asm",            // Compiler Comments
   [FLAG_STACK_INIT]          = "-fstack-init",             // Initalize stack to X
   [FLAG_RAM_MASK]            = "-fram-mask",               // Load / Stores always use RAM mask X
   [FLAG_ASM_COMMENT]         = "-fasm-comment-prefix",     // Compiler comments prefix
   [FLAG_HALF_SIZE]           = "-fhalf-size",              // Size of a half in address-space
   [FLAG_NO_INIT_ZERO_REG]    = "-fno-init-zero-reg",       // Don't load register x0 with 0
   [FLAG_NO_SYNTAX_ANALYZER]  = "-fno-syntax-analyzer",     // Dont analyze your code's syntax.
   // ISA extensions                                        /////////////////////// ISA extensions
   [FLAG_ZJREG_EXTENSION]     = "-Zjreg"                    // Enable compiler to use Zjreg extension
};

bool is_cflag_enabled(const char *cflag);
const char *get_cflag_value(const char *cflag);

#endif // CFLAGS_H