#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mrsh/parser.h>
#include <mrsh/ast.h>
int main(void){const char *source="echo hello";clock_t t=clock();for(int i=0;i<10000;i++){struct mrsh_parser *p=mrsh_parser_with_data(source,strlen(source));struct mrsh_program *a=mrsh_parse_program(p);if(!a||mrsh_parser_error(p,0))return 1;mrsh_program_destroy(a);mrsh_parser_destroy(p);}printf("%.6f\n",(double)(clock()-t)/CLOCKS_PER_SEC);}
