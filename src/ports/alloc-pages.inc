/* Invocation-owned small blocks. No resident arena: empty pages are returned
   immediately, and normal runtime cleanup releases all live/cached blocks.
   Native admission still enforces its emergency free-heap margin per page. */
typedef struct AllocPage AllocPage;
typedef struct Allocation {struct Allocation *prev,*next;size_t size;AllocPage *page;long double align;} Allocation;
struct AllocPage {AllocPage *next;Allocation *free;unsigned live,stride;long double align;};
static AllocPage *alloc_pages;
static unsigned long owned,peak;
#define ALLOC_PAGE_BYTES 2048
/* port_malloc checks header+size overflow before entering this helper. */
static Allocation *allocation_new(size_t n){
 size_t total=sizeof(Allocation)+n;
 if(total<=256){
  unsigned stride=total<=64?64:total<=128?128:256;
  AllocPage *page=alloc_pages;
  while(page&&(page->stride!=stride||!page->free))page=page->next;
  if(!page&&owned<=ULONG_MAX-ALLOC_PAGE_BYTES){
   page=cli_alloc(ALLOC_PAGE_BYTES);
   if(page){
    page->next=alloc_pages;alloc_pages=page;page->free=NULL;page->live=0;page->stride=stride;
    unsigned count=(ALLOC_PAGE_BYTES-sizeof(*page))/stride;
    for(unsigned i=0;i<count;i++){Allocation *p=(Allocation*)((char*)(page+1)+i*stride);p->next=page->free;page->free=p;}
    owned+=ALLOC_PAGE_BYTES;
   }
  }
  if(page){Allocation *p=page->free;page->free=p->next;page->live++;p->page=page;p->size=stride-sizeof(*p);return p;}
  /* A page may not fit under pressure while the exact request still does. */
 }
 if(owned>ULONG_MAX-total){errno=ENOMEM;return NULL;}
 Allocation *p=cli_alloc(total);if(p){p->page=NULL;p->size=n;owned+=total;}return p;
}
static void allocation_delete(Allocation *p){
 AllocPage *page=p->page;
 if(!page){owned-=sizeof(*p)+p->size;cli_free(p);return;}
 p->next=page->free;page->free=p;
 if(--page->live==0){
  AllocPage **link=&alloc_pages;while(*link!=page)link=&(*link)->next;
  *link=page->next;owned-=ALLOC_PAGE_BYTES;cli_free(page);
 }
}
