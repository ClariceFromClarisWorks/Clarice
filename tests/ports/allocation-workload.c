/* Shared runtime integration: caches, realloc, failure, cancellation and cleanup. */
#include "compat.h"
#include <stdint.h>
int upstream_main(int argc,char **argv){
 (void)argc;(void)argv;unsigned char *slots[128]={0};size_t sizes[128]={0};uint32_t state=68000;
 for(unsigned i=0;i<10000;i++){
  state=state*1664525+1013904223;unsigned at=(state>>16)%128;
  if(slots[at]){
   for(size_t j=0;j<sizes[at];j++)if(slots[at][j]!=(unsigned char)at)return 10;
   if(i&1){size_t n=sizes[at]+73;unsigned char *q=realloc(slots[at],n);if(!q)return 2;for(size_t j=0;j<sizes[at];j++)if(q[j]!=(unsigned char)at)return 11;memset(q,(unsigned char)at,n);slots[at]=q;sizes[at]=n;}
   else {free(slots[at]);slots[at]=NULL;}
  }else {size_t n=1+(state>>8)%400;slots[at]=malloc(n);if(!slots[at])return 2;sizes[at]=n;memset(slots[at],(unsigned char)at,n);}
  port_poll();
 }
 /* Deliberately return with live blocks: the invocation owns final cleanup. */
 return 0;
}
