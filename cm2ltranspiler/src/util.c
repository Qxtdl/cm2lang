#include "globals.h"
#include "util.h"

#include <string.h>
#include <stdbool.h>

char *r_strcat(char *dest, const char *src)
{
   if (!dest || !src) return NULL;
   dest = realloc(dest, strlen(dest) + strlen(src) + 1);
   return strcat(dest, src);
}

// char *strreplace(char *string, const char *target, const char *replacement)
// {
//    char *original_string = string;
//    char *new_string = NULL;
//    char *target_occurence;
//    while ((target_occurence = strstr(string, target))) {
//       //printf("to:%s\nstr:%s\n", target_occurence, string);
//       size_t string_len_before_target_occurence = target_occurence - string;
//       new_string = realloc(new_string, string_len_before_target_occurence + 1);
//       strncpy(new_string, string, string_len_before_target_occurence);
//       new_string[string_len_before_target_occurence] = '\0';
//       string = r_strcat(new_string, replacement);
//       strign = r_strcat(original_string + target_occurence)
//       //printf("string:%s\nnewstring:%s\n", string, new_string);
//    }
//    return string;
// }

char *strreplace(char *string, const char *target, const char *replacement)
{
   char *new_string = NULL;
   char *target_occurence;
   while ((target_occurence = strstr(string, target))) {
      char *rest = target_occurence + strlen(target);
      size_t string_len_before_target_occurence = target_occurence - string;
      new_string = realloc(new_string, string_len_before_target_occurence + 1);
      strncpy(new_string, string, string_len_before_target_occurence);
      new_string[string_len_before_target_occurence] = '\0';
      r_strcat(new_string, replacement);
      string = r_strcat(new_string, rest);
   }
   return string;
}

char *window_replace(char *string, const char *target, const char *replacement)
{
   if (strstr(string, target)) {

   }
}