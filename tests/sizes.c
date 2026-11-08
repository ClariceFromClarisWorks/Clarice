#include "sys6.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
int main(void){
 struct {unsigned long long bytes;const char *plain,*human;} cases[]={
 {0,"0","0B"},{35,"35","35B"},{1023,"1023","1023B"},{1024,"1024","1.0KiB"},
 {1536,"1536","1.5KiB"},{10240,"10240","10KiB"},{1048575,"1048575","1.0MiB"},
 {4294967295ULL,"4294967295","4.0GiB"},{4294967296ULL,"4294967296","4.0GiB"},
 {65535ULL*4294967295ULL,"281470681677825","256TiB"},
 {ULLONG_MAX,"18446744073709551615","16EiB"}};
 unsigned i;for(i=0;i<sizeof cases/sizeof cases[0];i++){char b[40];format_size(b,cases[i].bytes,0);assert(!strcmp(b,cases[i].plain));format_size(b,cases[i].bytes,1);if(strcmp(b,cases[i].human)){printf("%u: %s != %s\n",i,b,cases[i].human);return 1;}}
 puts("size formatter: 11 boundary cases, decimal and binary units PASS");return 0;
}
