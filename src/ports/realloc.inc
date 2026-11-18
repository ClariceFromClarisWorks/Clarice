/* Optional native resize, with ISO C failure preservation and copy fallback. */
#ifdef PORT_NATIVE_RESIZE
extern int port_native_resize(void *,unsigned long,unsigned long);
#endif
void *port_realloc(void *ptr,size_t n){
 if(!ptr)return port_malloc(n);
 if(!n){port_free(ptr);return NULL;}
 Allocation *p=(Allocation*)ptr-1;
 if(n>SIZE_MAX-sizeof(*p)){errno=ENOMEM;return NULL;}
#ifdef PORT_NATIVE_RESIZE
 port_poll();
 if(n==p->size||(p->page&&n<=p->size))return ptr;
 if(!p->page){
  int saved=errno;
  if(port_native_resize(p,sizeof(*p)+p->size,sizeof(*p)+n)){
   #ifdef PORT_ALLOC_TRACE
   resize_hits++;resize_saved+=n<p->size?n:p->size;
#endif
   owned=owned-p->size+n;p->size=n;if(owned>peak)peak=owned;
   errno=saved;return ptr;
  }
  /* Failed shrinking is still a valid realloc: retain the capacity/accounting. */
  if(n<p->size){errno=saved;return ptr;}
  if(spares){
   discard_spares();
   if(port_native_resize(p,sizeof(*p)+p->size,sizeof(*p)+n)){
    #ifdef PORT_ALLOC_TRACE
   resize_hits++;resize_saved+=n<p->size?n:p->size;
#endif
   owned=owned-p->size+n;p->size=n;if(owned>peak)peak=owned;
    errno=saved;return ptr;
   }
  }
  errno=saved;
 }
#endif
#ifdef PORT_ALLOC_TRACE
 realloc_old=p->size;realloc_request=n;
#endif
 void *q=port_malloc(n);
#ifdef PORT_ALLOC_TRACE
 realloc_old=realloc_request=0;
#endif
 if(q){
#ifdef PORT_ALLOC_TRACE
 resize_copies++;resize_copy_bytes+=n<p->size?n:p->size;
#endif
 memcpy(q,ptr,n<p->size?n:p->size);port_free(ptr);}
 return q;
}
