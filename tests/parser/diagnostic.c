#include <stdio.h>
#include <assert.h>
#include <mrsh/parser.h>
#include <mrsh/ast.h>
#include "fault.h"
static size_t run(const char *source,size_t fail,int sticky){
 pf_calls=pf_hit=0;pf_fail=fail;pf_sticky=sticky;
 struct mrsh_parser *p=mrsh_parser_with_data(source,strlen(source));
 struct mrsh_program *program=mrsh_parse_program(p);const char *error=mrsh_parser_error(p,NULL);
 if(pf_hit)assert(error&&!strcmp(error,"out of memory"));
 else assert(error&&strcmp(error,"out of memory"));
 size_t count=pf_calls;mrsh_program_destroy(program);mrsh_parser_destroy(p);assert(!pf_live);
 pf_fail=0;p=mrsh_parser_with_data("echo ok",7);program=mrsh_parse_program(p);assert(program&&!mrsh_parser_error(p,NULL));mrsh_program_destroy(program);mrsh_parser_destroy(p);assert(!pf_live);return count;
}
int main(void){const char *cases[]={"echo \"","echo '","echo ${x:-\"bad}","if true; then","for x in a; do","echo $(echo hi","echo ${}"};size_t trials=0;for(size_t i=0;i<sizeof cases/sizeof *cases;i++){size_t n=run(cases[i],0,0);for(int sticky=0;sticky<2;sticky++)for(size_t at=1;at<=n;at++){run(cases[i],at,sticky);trials++;}}printf("PASS %zu injected diagnostic failures; syntax vs OOM and subsequent recovery\n",trials);}
