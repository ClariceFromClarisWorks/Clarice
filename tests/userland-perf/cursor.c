/* Production parser cursor: refill, aliases, reset, lookahead and ownership. */
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"
#include <mrsh/ast.h>
static const char *alias(const char *s,void *ctx){(void)ctx;return !strcmp(s,"a")?"printf long-alias":NULL;}
static void parse(struct mrsh_parser*p){while(!mrsh_parser_eof(p)){struct mrsh_program*a=mrsh_parse_program(p);assert(a&&!mrsh_parser_error(p,0));mrsh_program_destroy(a);}}
int main(void){
 char *s=malloc(32769);for(int i=0;i<512;i++){memset(s+i*64,' ',64);memcpy(s+i*64,"a; : #",6);s[i*64+63]='\n';}s[32768]=0;
 struct mrsh_parser*p=mrsh_parser_with_data(s,32768);assert(p);p->alias=alias;parse(p);mrsh_parser_destroy(p);
 FILE*f=tmpfile();assert(f);assert(fwrite(s,1,32768,f)==32768);rewind(f);p=mrsh_parser_with_fd(fileno(f));assert(p);parse(p);assert(p->buf.cap<16384);mrsh_parser_destroy(p);fclose(f);
 struct mrsh_buffer b={0};assert(mrsh_buffer_append(&b,"echo one\necho two\n",18));p=mrsh_parser_with_buffer(&b);assert(p);struct mrsh_program*a=mrsh_parse_program(p);assert(a&&!mrsh_parser_error(p,0));mrsh_program_destroy(a);
 mrsh_parser_reset(p);assert(!p->read_pos&&!p->buf.len);assert(mrsh_buffer_append(&b,"echo reset\n",11));parse(p);mrsh_parser_destroy(p);mrsh_buffer_finish(&b);
 p=mrsh_parser_with_data(s,32768);char chunk[97];size_t n=0;while(n<32768){size_t want=32768-n;if(want>sizeof chunk)want=sizeof chunk;assert(parser_read(p,chunk,want)==want&&!memcmp(chunk,s+n,want));n+=want;}assert(p->read_pos==32768);mrsh_parser_destroy(p);free(s);puts("PASS cursor: data/fd/buffer, alias, refill, reset, chunked reads");
}
