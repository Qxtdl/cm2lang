#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../globals.h"
#include "transpiler.h"
#include "../parser/parser.h"

struct transpiler_state transpiler_state = {0};

void transpiler_init(char *asm_source)
{
   transpiler_state.transpiled = malloc(1);
   *transpiler_state.transpiled = '\0';
   transpiler_state.transpiled_len = 0;
   transpiler_state.asm_source = asm_source;
}

extern ast_node_t *program_node;

typedef struct {
   bool is_label;
   const char *label;

   const char *opcode;
   const char *rs1;
   const char *rs2;
   const char *rd;
} asm_line_t;

[[nodiscard]]
static asm_line_t get_asm_line(bool *get_asm_line_continue)
{
   static char *token = NULL, *token_saveptr;
   if (token == NULL) 
      token = __strtok_r(token = transpiler_state.asm_source, "\n", &token_saveptr);
   else
      token = __strtok_r(NULL, "\n", &token_saveptr);
   asm_line_t asm_line = {0};
   if (token == NULL) {
      *get_asm_line_continue = false;
      return asm_line;
   } else
      *get_asm_line_continue = true;
   if (strstr(token, ":")) {
      asm_line.is_label = true;
      asm_line.label = token;
      return asm_line;
   } else {
      asm_line.is_label = false;
      char *asm_token = token;
      asm_line.opcode = asm_token = strtok(asm_token, " ");
      asm_line.rd = asm_token = strtok(NULL, " ");
      asm_line.rs1 = asm_token = strtok(NULL, ",");
      asm_line.rs2 = asm_token = strtok(NULL, ", ");
      asm_line.opcode = strtok(asm_line.opcode, "\t");
      strtok(asm_line.rd, ",");
   }

   return asm_line;
}

#define r_strcat(src) \
if (src) { \
   transpiler_state.transpiled = realloc(transpiler_state.transpiled, strlen(transpiler_state.transpiled) + strlen((src)) + 1); \
   strcat(transpiler_state.transpiled, (src)); }

extern ast_node_t *registers_section_node;

static const char *mk_operand(asm_line_t asm_inst, const char *operand)
{
   if (!operand) return operand;

   const char *evaluated_operand;
   if (!strcmp(operand, "rd"))
      evaluated_operand = asm_inst.rd;
   else if (!strcmp(operand, "rs1") || !strcmp(operand, "imm"))
      evaluated_operand = asm_inst.rs1;
   else if (!strcmp(operand, "rs2") || !strcmp(operand, "imm2"))
      evaluated_operand = asm_inst.rs2;
   else evaluated_operand = operand;
   ast_node_t *vardecl_node;
   while ((vardecl_node = ast_see(&registers_section_node->node_union.section_node.body)))
      if (!strcmp(vardecl_node->node_union.asmdecl_node.name.ast_nodes[0]->node_union.name_node.value, evaluated_operand))
         evaluated_operand = vardecl_node->node_union.asmdecl_node.value.ast_nodes[0]->node_union.name_node.value;
   return evaluated_operand;
}

[[nodiscard]]
static bool transpiler_process_globals(ast_node_t *global_node)
{
   if (!global_node) return false;

   if (global_node->type == NODE_SECTION) {
      if (!strcmp(global_node->node_union.section_node.type.ast_nodes[0]->node_union.name_node.value, "cpu")) {
         ast_node_t *subsection_node;
         while ((subsection_node = ast_walk(&global_node->node_union.section_node.body))) {
            if (!strcmp(subsection_node->node_union.section_node.type.ast_nodes[0]->node_union.name_node.value, "instructions"))
            {
               bool get_asm_line_continue;
               asm_line_t asm_line;
               while (1) {
                  asm_line = get_asm_line(&get_asm_line_continue);
                  if (get_asm_line_continue == false) break;
                  
                  if (asm_line.is_label) {
                     r_strcat(asm_line.label)
                     r_strcat("\n");
                     continue;
                  }
                  ast_node_t *asmdecl_node;
                  while ((asmdecl_node = ast_see(&subsection_node->node_union.section_node.body))) {
                     if (!strcmp(asmdecl_node->node_union.asmdecl_node.name.ast_nodes[0]->node_union.name_node.value, asm_line.opcode)) {
                        char *inst = strdup(asmdecl_node->node_union.asmdecl_node.value.ast_nodes[0]->node_union.name_node.value);
                        strtok(inst, "#");
                        r_strcat("\t")
                        r_strcat(inst)
                        char *rd = mk_operand(asm_line, strtok(NULL, "#"));
                        r_strcat(rd)
                        char *rest = strtok(NULL, "#");
                        r_strcat(rest)
                        char *rs1 = mk_operand(asm_line, strtok(NULL, "#"));
                        r_strcat(rs1);
                        rest = strtok(NULL, "#");
                        r_strcat(rest);
                        char *rs2 = mk_operand(asm_line, strtok(NULL, "#"));
                        r_strcat(rs2);
                        r_strcat("\n");
                        free(inst); // stop leaksanitizer from complaining
                     }
                  }
               }
            }
         }
      }
   }

   return true;
}

void transpiler_process(void)
{
   while(transpiler_process_globals(ast_walk(&program_node->node_union.program_node.body)));
   transpiler_state.transpiled_len = strlen(transpiler_state.transpiled);
}