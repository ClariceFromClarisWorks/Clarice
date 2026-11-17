/* Experimental pre-shifted glyph contributions: 24,320 discardable bytes.
   The font remains the installed native Monaco 9. This is a derived cache,
   never the source of terminal or font state. Only one current bit phase is
   prepared; other phases rebuild or use the existing no-allocation path.
   begin/end surround a paint; invalidate after rebuilding native glyphs. */
#include <stdint.h>
#include "heap-admission.h"
#define GLYPH_PLANES_BYTES (4*95*64)
static Handle glyph_planes_handle;
static unsigned char *glyph_planes_data;
static int glyph_planes_phase=-1,glyph_planes_requested;
static unsigned long glyph_planes_retry;
static int glyph_planes_waiting;
extern void asm_ascii_planes_at(unsigned char *,const unsigned char *,const unsigned char *,long);
static void glyph_planes_invalidate(void){glyph_planes_phase=-1;}
static void glyph_planes_end(void){
 glyph_planes_data=0;
 if(glyph_planes_handle){HUnlock(glyph_planes_handle);HPurge(glyph_planes_handle);}
}
static void glyph_planes_dispose(void){
 glyph_planes_data=0;if(glyph_planes_handle){DisposeHandle(glyph_planes_handle);glyph_planes_handle=0;}
 glyph_planes_phase=-1;glyph_planes_waiting=0;
}
static int glyph_planes_begin(int phase){
 if(glyph_planes_data||phase<1||phase>7)return 0;
 glyph_planes_requested=phase;
 if(glyph_planes_handle&&!*glyph_planes_handle){DisposeHandle(glyph_planes_handle);glyph_planes_handle=0;glyph_planes_phase=-1;}
 if(!glyph_planes_handle){
  unsigned long now=TickCount();if(glyph_planes_waiting&&(unsigned long)(now-glyph_planes_retry)<60)return 0;
  glyph_planes_handle=native_heap_room(GLYPH_PLANES_BYTES,32768L,0)?NewHandle(GLYPH_PLANES_BYTES):0;
  if(!glyph_planes_handle){glyph_planes_waiting=1;glyph_planes_retry=now;return 0;}
  glyph_planes_waiting=0;glyph_planes_phase=-1;
 }
 HNoPurge(glyph_planes_handle);HLock(glyph_planes_handle);glyph_planes_data=(unsigned char*)*glyph_planes_handle;
 if(!glyph_planes_data){glyph_planes_end();return 0;}return 1;
}
static void glyph_planes_fill(unsigned char *planes,const unsigned char *glyphs,int phase){
 for(int p=0;p<4;p++)for(int c=0;c<95;c++){
  uint32_t *out=(uint32_t*)(planes+(p*95+c)*64);
  for(int y=0;y<11;y++)out[y]=(uint32_t)(glyphs[c*32+y]>>2)<<(26-phase-p*6);
 }
}
static int glyph_planes_prepare(const unsigned char *glyphs,int phase){
 if(!glyph_planes_data||phase!=glyph_planes_requested)return 0;
 if(glyph_planes_phase!=phase){glyph_planes_fill(glyph_planes_data,glyphs,phase);glyph_planes_phase=phase;}
 return 1;
}
