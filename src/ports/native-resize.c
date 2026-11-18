/* Optional classic Macintosh binding for invocation-owned NewPtr blocks.
 * SetPtrSize preserves the address; failure leaves the original block usable.
 * No OS patches, heap traversal, extra arena or partition change. */
#include <Memory.h>
#include <limits.h>
#include "../heap-admission.h"
int port_native_resize(void *block,unsigned long oldsize,unsigned long newsize){
 if(!block||newsize>LONG_MAX)return 0;
 /* Growth consumes only the delta. Keep exactly the allocator's recovery margin. */
 if(newsize>oldsize&&!native_heap_room(newsize-oldsize,32768L,1))return 0;
 /* Capture the documented register result directly. The pinned void trap
    wrapper discards it; the failure test found stale MemErr on this System 6 path. */
 register long result __asm__("d0")=(long)newsize;
 register void *pointer __asm__("a0")=block;
 __asm__ volatile(".word 0xA020":"+d"(result),"+a"(pointer)::"d1","a1","cc","memory");
 short error=(short)result;
#ifdef PORT_ALLOC_TRACE
 {static int reported;short saved=MemError();if(error!=saved&&!reported){
  extern long cli_write(int,const void *,unsigned long);extern int snprintf(char *,unsigned long,const char *,...);
  char b[140];int n=snprintf(b,sizeof b,"RESIZE_STATUS trap=%d MemErr=%d old=%lu new=%lu actual=%ld\n",error,saved,oldsize,newsize,(long)GetPtrSize((Ptr)block));
  if(n>0)cli_write(2,b,n);reported=1;
 }}
#endif
 return error==noErr;
}

/* Test-only actual allocation size; not linked into normal modules. */
long port_native_capacity(void *p){return GetPtrSize((Ptr)p);}
