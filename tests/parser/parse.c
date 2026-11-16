#include <stdio.h>
#include <mrsh/parser.h>
#include <mrsh/ast.h>
#include <mrsh/arithm.h>
#include "fault.h"

/* Faults cover only production parser allocations, including AST copies.
 * After teardown, parse again in the same process to verify recovery. */
static size_t measured_calls;
static int trial(const char *source, const char *mode, size_t at, int sticky) {
 pf_calls = pf_peak = pf_hit = 0;
 pf_sticky = sticky;
 int copying = !strcmp(mode, "copy"), arithmetic = !strcmp(mode, "arithmetic");
 pf_fail = copying ? 0 : at;
 struct mrsh_parser *p = mrsh_parser_with_data(source, strlen(source));
 struct mrsh_program *program = NULL, *copy = NULL;
 struct mrsh_arithm_expr *expr = NULL;
 if (arithmetic) expr = mrsh_parse_arithm_expr(p);
 else program = mrsh_parse_program(p);
 int error = mrsh_parser_error(p, NULL) != NULL;
 if (copying) {
  if (!program || error) return 2;
  pf_calls = 0; pf_fail = at;
  copy = mrsh_program_copy(program);
  error = !copy;
 }
 int bad = pf_hit && (copying ? copy != NULL : arithmetic ? expr != NULL : program != NULL);
 mrsh_program_destroy(copy); mrsh_program_destroy(program);
 mrsh_arithm_expr_destroy(expr); mrsh_parser_destroy(p);
 size_t calls = pf_calls, live = pf_live, peak = pf_peak;
 int hit = pf_hit;
 pf_fail = 0;
 p = mrsh_parser_with_data("echo recovered", 14);
 program = mrsh_parse_program(p);
 bad |= !program || mrsh_parser_error(p, NULL) != NULL;
 mrsh_program_destroy(program); mrsh_parser_destroy(p);
 measured_calls = calls;
 if (bad || live || pf_live || (hit && !error)) fprintf(stderr,"failure=%zu sticky=%d calls=%zu live=%zu hit=%d error=%d\n",at,sticky,calls,live,hit,error);
 return bad || live || pf_live || (hit && !error) ? 1 : 0;
}

int main(int argc, char **argv) {
 if (argc < 3) return 2;
 const char *mode = argc > 4 ? argv[4] : "parse";
 if (!strcmp(argv[1], "all")) {
  int rc = trial(argv[2], mode, 0, 0);
  if (rc) return rc;
  size_t count = measured_calls;
  for (int sticky = 0; sticky < 2; ++sticky)
   for (size_t at = 1; at <= count; ++at)
    if ((rc = trial(argv[2], mode, at, sticky))) return rc;
  printf("calls=%zu trials=%zu live=%zu\n",count,count*2,pf_live);
  return 0;
 }
 int rc=trial(argv[2],mode,strtoul(argv[1],NULL,10),argc>3&&!strcmp(argv[3],"sticky"));
 printf("calls=%zu live=%zu\n",measured_calls,pf_live);
 return rc;
}
