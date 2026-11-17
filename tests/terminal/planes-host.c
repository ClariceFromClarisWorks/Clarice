/* Exact cache lifecycle plus independent mathematical table oracle. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef char **Handle;static unsigned long ticks;static int fail,owned,locked,purgeable;static size_t bytes;
static unsigned long TickCount(void){return ticks;}
static long free_bytes=100000;
static long FreeMem(void){return free_bytes;}
static void PurgeMem(long n){(void)n;assert(!"cache must not request purging");}
static Handle NewHandle(long n){if(fail)return NULL;Handle h=malloc(sizeof *h);assert(h);*h=malloc(n);assert(*h);memset(*h,0xa5,n);bytes=n;owned++;return h;}
static void DisposeHandle(Handle h){free(*h);free(h);owned--;locked=purgeable=0;bytes=0;}
static void HLock(Handle h){assert(h&&*h);locked=1;}
static void HUnlock(Handle h){assert(h);locked=0;}
static void HPurge(Handle h){assert(h);purgeable=1;}
static void HNoPurge(Handle h){assert(h);purgeable=0;}
#include "../../src/glyph-planes.inc"
int main(void){
 unsigned char glyphs[95*32];for(int i=0;i<95*32;i++)glyphs[i]=(i*17&63)<<2;
 fail=1;assert(!glyph_planes_begin(3));assert(!owned);fail=0;assert(!glyph_planes_begin(3));ticks=60;
 for(int phase=1;phase<8;phase++){
  assert(glyph_planes_begin(phase));assert(locked&&!purgeable);assert(bytes==24320);assert(!glyph_planes_begin(phase));
  assert(!glyph_planes_prepare(glyphs,phase==7?1:phase+1));assert(glyph_planes_prepare(glyphs,phase));
  for(int p=0;p<4;p++)for(int c=0;c<95;c++)for(int y=0;y<11;y++){
   uint32_t expected=0;for(int bit=0;bit<6;bit++)if(glyphs[c*32+y]&(128>>bit))expected|=UINT32_C(1)<<(31-phase-p*6-bit);
   assert(((uint32_t*)(glyph_planes_data+(p*95+c)*64))[y]==expected);
  }
  glyph_planes_end();assert(!locked&&purgeable);assert(!glyph_planes_prepare(glyphs,phase));
 }
 assert(glyph_planes_begin(3));assert(glyph_planes_prepare(glyphs,3));glyphs[0]^=128;glyph_planes_invalidate();assert(glyph_planes_prepare(glyphs,3));assert(*(uint32_t*)glyph_planes_data==(uint32_t)(glyphs[0]>>2)<<23);glyph_planes_end();
 free(*glyph_planes_handle);*glyph_planes_handle=NULL;assert(glyph_planes_begin(3));assert(glyph_planes_phase==-1);assert(glyph_planes_prepare(glyphs,3));glyph_planes_end();
 for(int i=0;i<1000;i++){assert(glyph_planes_begin(3));assert(glyph_planes_prepare(glyphs,3));glyph_planes_end();}assert(owned==1);
 glyph_planes_dispose();assert(!owned&&!bytes);assert(!glyph_planes_begin(0));assert(!glyph_planes_begin(8));
 free_bytes=32768+GLYPH_PLANES_BYTES-1;ticks+=60;assert(!glyph_planes_begin(3)&&!owned);free_bytes++;ticks+=60;assert(glyph_planes_begin(3));glyph_planes_end();glyph_planes_dispose();assert(!owned);
 puts("PASS cache reserve admission; 29,260 glyph-plane oracle cases; failed allocation/retry, purge, invalidation, phase fallback, ownership and 1000 paint acquisitions;24320 bytes");
}
