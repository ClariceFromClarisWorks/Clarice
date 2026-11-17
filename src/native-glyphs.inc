extern void asm_ascii_planes_unrolled(unsigned char*,const unsigned char*,const unsigned char*,long);
#include "history-raster-cache.inc"
/* Diagnostic experiment: parent sets line identity immediately before each
   native_glyph_row call, and releases the locked handle once after painting. */
static int history_raster_enabled;
static long history_raster_line=-1;
static unsigned long history_raster_hits,history_raster_misses;
/* Monaco 9 ASCII cells share the unused part of the existing 4096-byte row
   staging bitmap. No allocation: first 704 bytes remain the 11-pixel output
   row; 95 * 32 glyph bytes occupy bytes 704..3743. Editor drawing may use the
   whole bitmap, so draw() invalidates these cells before handing it over. */
/* Four six-pixel cells overwrite three bytes. Only the OR-composed tail needs
   clearing; presentation clips to count*6 pixels, never to the unused padding. */
extern void asm_ascii_cells(unsigned char *,const unsigned char *,const unsigned char *,long,long,long);
#ifdef CLARICE_DRAW_DIAGNOSTICS
static int jump_legacy;
static int clear_legacy;
static int direct_legacy;
#define DIRECT_LEGACY direct_legacy
static unsigned long glyph_build_ticks;
#define JUMP_LEGACY jump_legacy
#else
#define JUMP_LEGACY 0
#define DIRECT_LEGACY 0
#endif
static void native_glyph_clear(unsigned char *output,int count){
#ifdef CLARICE_DRAW_DIAGNOSTICS
 if(clear_legacy){memset(output,0,STRIDE*11);return;}
#endif
 int tail=count&3;
 if(tail){unsigned char *p=output+(count>>2)*3;
  for(int y=0;y<11;y++,p+=STRIDE){p[0]=0;if(tail>1)p[1]=0;if(tail>2)p[2]=0;}
 }
}
extern void asm_ascii_groups_at(unsigned char *,const unsigned char *,const unsigned char *,long,long);
extern void asm_ascii_cells_at(unsigned char *,const unsigned char *,const unsigned char *,long,long,long,long);
static void native_glyph_compose_at(unsigned char *output,const char *s,int count,int phase){
 unsigned char *p=output+(phase>>3),*glyphs=prompt_pixels+STRIDE*11;
 int bits=phase&7,full=count&~3,start=(full>>2)*3,end=(bits+count*6+7)>>3;
 /* Full groups overwrite their interior; only the first byte and tail can
    combine with prior contents. Clear those before either compositor runs. */
 for(int y=0;y<11;y++){unsigned char *row=p+y*STRIDE;if(bits)row[0]=0;for(int i=start;i<end;i++)row[i]=0;}
 if(bits){
  if(full){if(planes_enabled&&!glyph_planes_data&&bits==((TEXT_LEFT-win->portBits.bounds.left)&7))glyph_planes_begin(bits);
 if(planes_enabled&&glyph_planes_prepare(glyphs,bits))(planes_enabled==2?asm_ascii_planes_unrolled:asm_ascii_planes_at)(p,(const unsigned char*)s,glyph_planes_data,full);else asm_ascii_groups_at(p,(const unsigned char*)s,glyphs,full,bits);}
  if(count!=full)asm_ascii_cells_at(p+start,(const unsigned char*)s+full,glyphs,count-full,11,6,bits);
 }else asm_ascii_cells(p,(const unsigned char*)s,glyphs,count,11,6);
}
static int native_glyph_row_to(unsigned char *output,const char *s,int count,int phase){
 if(JUMP_LEGACY||!mono||font_id!=4||font_size!=9||cell_w!=6||line_h!=11||font_ascent!=9||count<1||count>85||phase<0||phase>15||phase+count*6>512)return 0;
 for(int i=0;i<count;i++)if((unsigned char)s[i]<32||(unsigned char)s[i]>126)return 0;
 unsigned char *glyphs=prompt_pixels+STRIDE*11;
 unsigned char history_key[HISTORY_CACHE_KEY_BYTES];int cacheable=output==prompt_pixels&&history_raster_enabled&&scrollback>0&&count==columns&&history_raster_line>=0;
 /* Rebuilt native font cells also invalidate their derived history pixels. */
 if(!native_glyph_valid)history_cache_dispose();
 if(cacheable){
  memset(history_key,0,sizeof history_key);memcpy(history_key,s,count);
  /* Diagnostic and clipped-row callers can change geometry inside one paint.
     Never treat a held handle as proof that its pixels match this phase. */
  if(history_cache_rows&&(history_cache_columns!=columns||history_cache_phase!=phase||history_cache_font!=font_id||history_cache_size!=font_size))history_cache_end();
  if(!history_cache_rows)history_cache_begin(columns,phase,font_id,font_size,cell_w,line_h);
  if(history_cache_load(history_raster_line,history_key,prompt_pixels)){history_raster_hits++;goto history_ink_ready;}
  history_raster_misses++;
 }
 if(!native_glyph_valid){glyph_planes_invalidate();
  DRAW_STAMP(build_start);
  ForeColor(blackColor);BackColor(whiteColor);
  for(int first=32;first<127;){
   char chars[85];int n=127-first;if(n>85)n=85;
   for(int i=0;i<n;i++)chars[i]=(char)(first+i);
   Rect r;SetRect(&r,0,0,n*6,11);EraseRect(&r);MoveTo(0,9);DrawText(chars,0,n);
   for(int i=0;i<n;i++){
    int bit=i*6,byte=bit>>3,shift=bit&7;unsigned char *g=glyphs+(first+i-32)*32;
    for(int y=0;y<11;y++){
     const unsigned char *p=prompt_pixels+y*STRIDE+byte;
     unsigned bits=(unsigned)*p<<8;if(shift>2)bits|=p[1];
     g[y]=(bits>>(8-shift))&0xfc;
    }
   }
   first+=n;
  }
  ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);
  native_glyph_valid=1;
#ifdef CLARICE_DRAW_DIAGNOSTICS
  glyph_build_ticks+=TickCount()-build_start;
#endif
 }
 if(phase)native_glyph_compose_at(output,s,count,phase);
 else{native_glyph_clear(output,count);asm_ascii_cells(output,(const unsigned char*)s,glyphs,count,11,6);}
 if(cacheable)history_cache_store(history_raster_line,history_key,prompt_pixels);
history_ink_ready:
 return 1;
}
static int native_glyph_row(const char *s,int count,int phase){
 if(!native_glyph_row_to(prompt_pixels,s,count,phase))return 0;
 if(inverse){Rect r;SetRect(&r,phase,0,phase+count*6,11);InvertRect(&r);}
 return 1;
}
