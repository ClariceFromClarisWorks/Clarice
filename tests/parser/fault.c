#define PF_IMPLEMENTATION
#include "fault.h"
#include <stdio.h>
struct allocation {struct allocation *next;void *p;size_t n;};
static struct allocation *list;
size_t pf_calls,pf_live,pf_peak,pf_fail;int pf_hit,pf_sticky;
static int deny(void){pf_calls++;if(pf_fail&&(pf_calls==pf_fail||(pf_sticky&&pf_calls>=pf_fail))){pf_hit=1;return 1;}return 0;}
void *pf_malloc(size_t n){if(deny())return NULL;void *p=malloc(n);if(p){struct allocation *a=malloc(sizeof *a);if(!a)abort();*a=(struct allocation){list,p,n};list=a;pf_live+=n;if(pf_live>pf_peak)pf_peak=pf_live;}return p;}
void *pf_calloc(size_t n,size_t s){if(s&&n>(size_t)-1/s)return NULL;void *p=pf_malloc(n*s);if(p)memset(p,0,n*s);return p;}
void pf_free(void *p){if(!p)return;struct allocation **a=&list;while(*a&&(*a)->p!=p)a=&(*a)->next;if(!*a){fprintf(stderr,"unowned free\n");abort();}struct allocation *old=*a;*a=old->next;pf_live-=old->n;free(old);free(p);}
void *pf_realloc(void *p,size_t n){if(!p)return pf_malloc(n);if(deny())return NULL;struct allocation *a=list;while(a&&a->p!=p)a=a->next;if(!a)abort();void *q=realloc(p,n);if(q){pf_live-=a->n;a->n=n;a->p=q;pf_live+=n;if(pf_live>pf_peak)pf_peak=pf_live;}return q;}
char *pf_strndup(const char *s,size_t n){size_t k=strnlen(s,n);char *p=pf_malloc(k+1);if(p){memcpy(p,s,k);p[k]=0;}return p;}
char *pf_strdup(const char *s){return pf_strndup(s,strlen(s));}
