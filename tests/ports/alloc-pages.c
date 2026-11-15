/* Actual pool implementation, randomized ownership/content and low-memory tests. */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stddef.h>
static unsigned long native_bytes,budget=ULONG_MAX,calls;
typedef union {max_align_t align;size_t size;} Header;
static void *cli_alloc(unsigned long n){calls++;if(n>budget||native_bytes>budget-n)return NULL;Header *p=malloc(sizeof(*p)+n);assert(p);p->size=n;native_bytes+=n;return p+1;}
static void cli_free(void *p){Header *h=(Header*)p-1;native_bytes-=h->size;free(h);}
#include "../../src/ports/alloc-pages.inc"
static uint32_t seed=68000;
static unsigned rnd(void){seed=seed*1664525+1013904223;return seed;}
int main(void){
 Allocation *slots[512]={0};size_t sizes[512]={0};
 for(unsigned i=0;i<50000;i++){
  unsigned at=rnd()%512;
  if(slots[at]){for(size_t j=0;j<sizes[at];j++)assert(((unsigned char*)(slots[at]+1))[j]==(unsigned char)at);allocation_delete(slots[at]);slots[at]=NULL;}
  else {size_t n=1+rnd()%700;Allocation *p=allocation_new(n);assert(p);assert((uintptr_t)(p+1)%_Alignof(max_align_t)==0);slots[at]=p;sizes[at]=n;memset(p+1,(unsigned char)at,n);}
  assert(owned==native_bytes);
 }
 for(unsigned i=0;i<512;i++)if(slots[i])allocation_delete(slots[i]);
 assert(!owned&&!native_bytes&&!alloc_pages);
 budget=sizeof(Allocation)+8;Allocation *p=allocation_new(8);assert(p&&!p->page);allocation_delete(p);
 budget=0;assert(!allocation_new(1));assert(!owned&&!alloc_pages);
 budget=ULONG_MAX;calls=0;for(unsigned i=0;i<512;i++)slots[i]=allocation_new(8);
 assert(calls<64);for(unsigned i=0;i<512;i++)allocation_delete(slots[i]);assert(!native_bytes&&!owned&&!alloc_pages);
 puts("PASS: 50000 mixed operations, content/alignment, exact-request fallback, exhaustion, native/owned accounting, page release and batching");
}
