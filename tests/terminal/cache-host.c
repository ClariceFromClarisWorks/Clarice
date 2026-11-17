/* Host ownership, invalidation and pressure tests for the exact cache source.
   Pixel composition/presentation remains covered by the native glyph tests. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
typedef char **Handle;typedef long Size;
static unsigned long ticks;static int fail,allocations,locked,purgeable;static size_t bytes;
static unsigned long TickCount(void){return ticks;}
static Handle NewHandle(Size n){if(fail)return NULL;Handle h=malloc(sizeof *h);assert(h);*h=malloc(n);assert(*h);memset(*h,0xa5,n);allocations++;bytes=n;return h;}
static void DisposeHandle(Handle h){free(*h);free(h);allocations--;locked=purgeable=0;bytes=0;}
static void HLock(Handle h){assert(h&&*h);locked=1;}
static void HUnlock(Handle h){assert(h);locked=0;}
static void HPurge(Handle h){assert(h);purgeable=1;}
static void HNoPurge(Handle h){assert(h);purgeable=0;}
static void BlockMove(const void *a,void *b,long n){memmove(b,a,n);}
#include "../../src/history-raster-cache.inc"
static int begin(void){return history_cache_begin(80,3,4,9,6,11);}
int main(void){
 unsigned char key[132]={0},other[132]={0},pixels[704],out[704];memset(pixels,0x96,sizeof pixels);key[0]='A';other[0]='B';
 assert(!history_cache_load(0,key,out));fail=1;assert(!begin());assert(!allocations);fail=0;assert(!begin());ticks=60;
 assert(begin());assert(locked&&!purgeable);assert(bytes<=72*1024);assert(!history_cache_load(1,key,out));
 history_cache_store(1,key,pixels);assert(history_cache_load(1,key,out));assert(!memcmp(out,pixels,704));assert(!history_cache_load(1,other,out));
 assert(!begin()); /* Nested entry cannot steal ownership. */
 history_cache_end();assert(!locked&&purgeable);assert(!history_cache_load(1,key,out));
 assert(begin());assert(history_cache_load(1,key,out));history_cache_store(1+HISTORY_CACHE_SLOTS,other,pixels);assert(!history_cache_load(1,key,out));history_cache_end();
 assert(history_cache_begin(79,3,4,9,6,11));assert(!history_cache_load(1+HISTORY_CACHE_SLOTS,other,out));history_cache_store(1,key,pixels);history_cache_end();
 assert(history_cache_begin(79,4,4,9,6,11));assert(!history_cache_load(1,key,out));history_cache_end();
 assert(!history_cache_begin(80,3,0,9,6,11));assert(!history_cache_begin(85,15,4,9,6,11));assert(!locked&&purgeable);
 /* Simulate Memory Manager purging unlocked data, retaining the master pointer. */
 free(*history_cache_handle);*history_cache_handle=NULL;assert(begin());assert(!history_cache_load(1,key,out));history_cache_end();
 for(int i=0;i<1000;i++){assert(begin());history_cache_store(i,key,pixels);assert(history_cache_load(i,key,out));history_cache_end();}assert(allocations==1);
 size_t peak=bytes;history_cache_dispose();assert(!allocations&&!bytes);assert(!history_cache_load(1,key,out));
 printf("PASS cache hit/miss, exact key, collision, geometry, purge, allocation failure/retry, ownership and 1000 repeated acquisitions; host storage=%zu bytes\n",peak);
}
