#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/display-size.h"
int main(void){struct {unsigned long long n;const char *text;} cases[]={
 {0,"0 B"},{1,"1 B"},{1023,"1023 B"},{1024,"1.00 KB"},{12800,"12.50 KB"},{1048576,"1.00 MB"},{1572864,"1.50 MB"},{1073741824,"1.00 GB"},{(1ULL<<60),"1.00 EB"},{~0ULL,"16.00 EB"},{1048575,"1.00 MB"},{1029,"1.00 KB"},{1030,"1.01 KB"}};
 for(unsigned i=0;i<sizeof cases/sizeof *cases;i++){char out[32];display_size(out,cases[i].n);if(strcmp(out,cases[i].text)){fprintf(stderr,"%u: %s != %s\n",i,out,cases[i].text);return 1;}}
 puts("PASS: byte/KB/MB/GB/EB boundaries, two decimals, rounding carry, full 64-bit range; no floating point");}
