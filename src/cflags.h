#ifndef CFLAGS_H
#define CFLAGS_H

#include <stdbool.h>

#define FLAG_VERBOSE_ASM 0
#define FLAG_STACK_INIT 1
#define FLAG_RAM_MASK 2
#define FLAG_ASM_COMMENT 3
#define FLAG_HALF_SIZE 4
#define FLAG_NO_INIT_ZERO_REG 5

static const char * const cflags[] = {
   [FLAG_VERBOSE_ASM]      = "-fverbose-asm",
   [FLAG_STACK_INIT]       = "-fstack-init",
   [FLAG_RAM_MASK]         = "-fram-mask",
   [FLAG_ASM_COMMENT]      = "-fasm-comment",
   [FLAG_HALF_SIZE]        = "-fhalf-size",
   [FLAG_NO_INIT_ZERO_REG] = "-fno-init-zero-reg"
};

bool is_cflag_enabled(const char *cflag);
const char *get_cflag_value(const char *cflag);

#endif // CFLAGS_H