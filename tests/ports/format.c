/* Exercise the shared newlib format adapter, including nano's va_list path.
 * Run with runtime.c and dead-section elimination, or as a CLX command. */
#include "../../src/ports/compat.h"
#include <assert.h>
#include <stddef.h>
static int via_va(char *out,size_t n,const char *fmt,...){
 va_list ap;va_start(ap,fmt);int r=vsnprintf(out,n,fmt,ap);va_end(ap);return r;
}
static int via_unbounded_va(char *out,const char *fmt,...){
 va_list ap;va_start(ap,fmt);int r=vsprintf(out,fmt,ap);va_end(ap);return r;
}
int main(void){
 char out[128],small[5];
 assert(via_va(out,sizeof out,"Read %zu lines",(size_t)128)==14);
 assert(!strcmp(out,"Read 128 lines"));
 assert(via_va(out,sizeof out,"Read %zu line",(size_t)1)==11);
 assert(!strcmp(out,"Read 1 line"));
 sprintf(out,"%zi,%zi",(size_t)127,(size_t)4097);
 assert(!strcmp(out,"127,4097"));
 via_unbounded_va(out,"%zu %td %%zu %08zu",(size_t)8192,(ptrdiff_t)-3,(size_t)42);
 assert(!strcmp(out,"8192 -3 %zu 00000042"));
 assert(via_va(small,sizeof small,"Read %zu lines",(size_t)128)==14);
 assert(!strcmp(small,"Read"));
 assert(via_va(NULL,0,"%zu",(size_t)8192)==4);
 snprintf(out,sizeof out,"%zu %s %d",(size_t)128,"lines",-7);
 assert(!strcmp(out,"128 lines -7"));
 return 0;
}
