
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mrsh/parser.h>
#include <mrsh/ast.h>
extern unsigned long long moved;
int main(void){size_t sizes[]={64,8192,32768};for(int k=0;k<3;k++){size_t n=sizes[k];char *s=malloc(n+1);memset(s,' ',n);for(size_t i=0;i<n;i+=64){s[i]=':';s[i+1]=' ';s[i+2]='#';s[i+63]='\n';}s[n]=0;moved=0;clock_t t=clock();for(int i=0;i<20;i++){struct mrsh_parser*p=mrsh_parser_with_data(s,n);while(!mrsh_parser_eof(p)){struct mrsh_program*a=mrsh_parse_program(p);if(!a||mrsh_parser_error(p,0))return 1;mrsh_program_destroy(a);}mrsh_parser_destroy(p);}printf("%zu bytes: %.6f host CPU seconds/parse, %llu moved bytes/parse\n",n,(double)(clock()-t)/CLOCKS_PER_SEC/20,moved/20);free(s);}return 0;}
