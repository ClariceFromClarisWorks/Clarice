/* Host-only accounting/fault injection for adapter allocations (not mrsh). */
struct host_alloc {struct host_alloc *next;void *ptr;size_t size;};
static struct host_alloc *host_allocs;
static size_t host_live,host_peak,host_calls;static int host_fault;
static int host_deny(void){const char *s=getenv("CLARICE_SHELL_FAIL_ALLOC");host_calls++;if(s&&strtoul(s,0,10)==host_calls){host_fault=1;return 1;}return 0;}
static void *host_malloc(size_t size){if(host_deny())return 0;void *p=malloc(size);if(p){struct host_alloc *a=malloc(sizeof *a);if(!a){free(p);return 0;}a->ptr=p;a->size=size;a->next=host_allocs;host_allocs=a;host_live+=size;if(host_live>host_peak)host_peak=host_live;}return p;}
static void host_free(void *p){struct host_alloc **a=&host_allocs;while(*a){if((*a)->ptr==p){struct host_alloc *old=*a;*a=old->next;host_live-=old->size;free(old);break;}a=&(*a)->next;}free(p);}
static void *host_calloc(size_t n,size_t size){if(size&&n>(size_t)-1/size)return 0;void *p=host_malloc(n*size);if(p)memset(p,0,n*size);return p;}
static void *host_realloc(void *p,size_t size){if(!p)return host_malloc(size);struct host_alloc *a=host_allocs;while(a&&a->ptr!=p)a=a->next;if(!a){fprintf(stderr,"untracked realloc\n");abort();}if(host_deny())return 0;void *q=realloc(p,size);if(q){host_live-=a->size;host_live+=size;a->ptr=q;a->size=size;if(host_live>host_peak)host_peak=host_live;}return q;}
#define malloc host_malloc
#define calloc host_calloc
#define realloc host_realloc
#define free host_free
