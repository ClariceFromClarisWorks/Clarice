#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>
int port_awk_integer(char *,size_t,const char *,uintmax_t);
int main(void){const char *fmts[]={"%jd","%+08jd","%-10jd","%.0jd","%12.6jd","%#jo","%#020jx","%#jX","%ju","% .4jd","%#.0jo","%#.0jx"};uintmax_t values[]={0,1,7,42,255,2147483648ULL,4294967295ULL,UINT64_MAX,UINT64_C(0x8000000000000000)};for(unsigned f=0;f<sizeof fmts/sizeof fmts[0];f++)for(unsigned v=0;v<sizeof values/sizeof values[0];v++)for(unsigned cap=0;cap<50;cap++){char a[80],b[80];memset(a,0,sizeof a);memset(b,0,sizeof b);int n=port_awk_integer(a,cap,fmts[f],values[v]);int m=snprintf(b,cap,fmts[f],values[v]);assert(n==m);assert(!memcmp(a,b,cap));}puts("PASS: 5400 intmax format/truncation comparisons");}
