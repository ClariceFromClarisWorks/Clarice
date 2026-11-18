/* Real 68000 realloc semantics and cleanup, native Memory Manager included. */
#include "compat.h"
#include <stdint.h>
extern long port_native_capacity(void *);
typedef struct Header {struct Header *prev,*next;size_t size;void *page;long double align;} Header;
static int valid(void *p,size_t n){Header *h=(Header*)p-1;if(h->size<n)return 0;if(!h->page&&port_native_capacity(h)<(long)(sizeof(*h)+h->size))return 0;return 1;}
static int same(const unsigned char *p,size_t n,unsigned char v){for(size_t j=0;j<n;j++)if(p[j]!=v)return 0;return 1;}
int upstream_main(int argc,char **argv){
 puts("ALLOC_TEST begin");fflush(stdout);unsigned char *p=malloc(8192);if(!p)return 10;memset(p,37,8192);
 unsigned char *q=realloc(p,4096);if(q!=p||!same(q,4096,37))return 11;
 q=realloc(p,32768);if(!q||!same(q,4096,37))return 12;p=q;
 errno=0;q=realloc(p,1024UL*1024UL);if(q||errno!=ENOMEM||!same(p,4096,37))return 13;
 q=realloc(p,SIZE_MAX);if(q||errno!=ENOMEM||!same(p,4096,37))return 14;
 if(realloc(p,0))return 15;
 p=realloc(NULL,8);if(!p)return 16;memset(p,73,8);q=realloc(p,4);if(q!=p||!same(q,4,73))return 17;free(q);
 puts("ALLOC_TEST edge cases passed");fflush(stdout);unsigned char *slot[32]={0};size_t sizes[32]={0};uint32_t state=68000;
 for(unsigned i=0;i<2000 || (argc==2&&!strcmp(argv[1],"cancel"));i++){
  state=state*1664525+1013904223;unsigned at=(state>>16)%32;size_t n=1+(state%1500);
  if(i%10==0){printf("ALLOC_TEST iteration %u\n",i);fflush(stdout);}
  if(slot[at]&&(!valid(slot[at],sizes[at])||!same(slot[at],sizes[at],at)))return 20;
  q=realloc(slot[at],n);if(!q)return 21;if(!valid(q,n)){printf("BAD CAPACITY i=%u n=%lu stored=%lu native=%ld\n",i,(unsigned long)n,(unsigned long)((Header*)q-1)->size,port_native_capacity((Header*)q-1));return 23;}
  if(slot[at]&&!same(q,n<sizes[at]?n:sizes[at],at))return 22;
  memset(q,at,n);slot[at]=q;sizes[at]=n;port_poll();
 }
 puts("PASS native realloc content, shrink, grow, failed-grow preservation, NULL/zero/overflow and 2000 mixed operations");
 return 0; /* Invocation cleanup must release deliberately retained blocks. */
}
