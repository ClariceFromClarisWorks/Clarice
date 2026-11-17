/* Opt-in guest diagnostic only. No hooks or allocation overhead in normal builds. */
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
static unsigned long calls,fail,live;
static int sticky,hit;
struct block {size_t size;};
void parser_test_begin(unsigned long at,int persistent){calls=hit=0;fail=at;sticky=persistent;}
void parser_test_end(unsigned long *count,unsigned long *bytes,int *failed){*count=calls;*bytes=live;*failed=hit;fail=0;}
static int deny(void){calls++;if(fail&&(calls==fail||(sticky&&calls>=fail))){hit=1;return 1;}return 0;}
void *parser_test_malloc(size_t size){if(size>(size_t)-1-sizeof(struct block)||deny())return NULL;struct block *b=malloc(sizeof *b+size);if(!b)return NULL;b->size=size;live+=size;return b+1;}
void parser_test_free(void *p){if(p){struct block *b=(struct block*)p-1;live-=b->size;free(b);}}
void *parser_test_calloc(size_t n,size_t size){if(size&&n>(size_t)-1/size)return NULL;void *p=parser_test_malloc(n*size);if(p)memset(p,0,n*size);return p;}
void *parser_test_realloc(void *p,size_t size){if(!p)return parser_test_malloc(size);if(size>(size_t)-1-sizeof(struct block)||deny())return NULL;struct block *old=(struct block*)p-1;size_t previous=old->size;struct block *b=realloc(old,sizeof *b+size);if(!b)return NULL;b->size=size;live=live-previous+size;return b+1;}
char *parser_test_strndup(const char *s,size_t n){size_t k=0;while(k<n&&s[k])k++;char *p=parser_test_malloc(k+1);if(p){memcpy(p,s,k);p[k]=0;}return p;}
char *parser_test_strdup(const char *s){return parser_test_strndup(s,strlen(s));}
