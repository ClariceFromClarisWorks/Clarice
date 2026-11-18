"""ASan/UBSan against actual allocation and realloc implementation, native calls mocked."""
from pathlib import Path
import tempfile,subprocess
r=Path(__file__).resolve().parents[2]
s=(r/'src/ports/runtime.c').read_text();body=s[s.index('static void discard_spares'):s.index('\nchar *port_strndup')]
prefix=r'''
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define PORT_NATIVE_RESIZE 1
#define allocation_failure(n) ((void)0)
typedef union {max_align_t a;struct {size_t n,cap;} s;} Native;
static size_t live,budget=SIZE_MAX;static int allow_resize=1;
static void *cli_alloc(unsigned long n){if(n>budget||live>budget-n)return NULL;Native *h=malloc(sizeof(*h)+n+131072);assert(h);h->s.n=n;h->s.cap=n+131072;live+=n;return h+1;}
static void cli_free(void *p){Native *h=(Native*)p-1;live-=h->s.n;free(h);}
int port_native_resize(void *p,unsigned long old,unsigned long n){Native *h=(Native*)p-1;assert(h->s.n==old);if(!allow_resize||n>h->s.cap||(n>old&&(n-old>budget||live>budget-(n-old))))return 0;live=live-old+n;h->s.n=n;return 1;}
#include "alloc-pages.inc"
static Allocation *allocations,*spares;static unsigned long spare_bytes;static int cleaning;
static void port_poll(void){}
void port_free(void*);
'''
tests=r'''
static void check(unsigned char *p,size_t n,unsigned char value){for(size_t i=0;i<n;i++)assert(p[i]==value);}
static void clear(void){cleaning=1;while(allocations)port_free(allocations+1);discard_spares();cleaning=0;assert(!owned&&!live&&!alloc_pages);}
int main(void){
 unsigned char *p=port_realloc(NULL,1024);assert(p);memset(p,37,1024);size_t before=owned;
 unsigned char *q=port_realloc(p,8192);assert(q==p);check(q,1024,37);assert(owned==before+7168&&owned==live);
 q=port_realloc(p,256);assert(q==p);check(q,256,37);assert(owned==before-768&&owned==live);
 allow_resize=0;before=owned;q=port_realloc(p,128);assert(q==p&&owned==before);check(q,128,37);
 q=port_realloc(p,4096);assert(q&&q!=p);check(q,256,37);p=q;
 budget=live;errno=0;q=port_realloc(p,262144);assert(!q&&errno==ENOMEM);check(p,256,37);assert(owned==live);
 q=port_realloc(p,SIZE_MAX);assert(!q&&errno==ENOMEM);check(p,256,37);
 assert(!port_realloc(p,0));clear();budget=SIZE_MAX;allow_resize=1;
 p=port_malloc(8);memset(p,91,8);q=port_realloc(p,4);assert(p==q);check(q,4,91);clear();
 unsigned char *slot[64]={0};size_t sizes[64]={0};uint32_t seed=68000;
 for(unsigned k=0;k<10000;k++){seed=seed*1664525+1013904223;unsigned at=(seed>>16)%64;size_t n=1+(seed%3000);allow_resize=!!(seed&8);
  if(slot[at])check(slot[at],sizes[at],at);
  q=port_realloc(slot[at],n);assert(q);if(slot[at])check(q,n<sizes[at]?n:sizes[at],at);memset(q,at,n);slot[at]=q;sizes[at]=n;assert(owned==live);
  if(k%7==0){port_free(slot[at]);slot[at]=NULL;sizes[at]=0;}
 }
 clear();puts("PASS in-place growth/shrink, shrink failure, copy fallback, allocation failure preserves original, NULL/zero/overflow, page blocks, 10000 mixed reallocs and cleanup");
}
'''
with tempfile.TemporaryDirectory() as td:
 p=Path(td);(p/'test.c').write_text(prefix+body+tests)
 subprocess.run(['clang','-std=c11','-g','-fsanitize=address,undefined','-I'+str(r/'src/ports'),str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
