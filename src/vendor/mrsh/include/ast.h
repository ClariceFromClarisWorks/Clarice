#ifndef AST_H
#define AST_H

#include <mrsh/ast.h>

void word_array_finish(struct mrsh_array *words);
void redirect_array_finish(struct mrsh_array *redirects);
void assignment_array_finish(struct mrsh_array *assignments);
void command_list_array_finish(struct mrsh_array *cmds);
void case_item_destroy(struct mrsh_case_item *item);

#endif
