/* Monaco 9 ASCII cells share the unused part of the existing 4096-byte row
   staging bitmap. No allocation: first 704 bytes remain the 11-pixel output
   row; 95 * 32 glyph bytes occupy bytes 704..3743. Editor drawing may use the
   whole bitmap, so draw() invalidates these cells before handing it over. */
/* Compositor writes a zeroed row; four six-pixel cells share three bytes. */
extern void asm_ascii_cells(unsigned char *,const unsigned char *,const unsigned char *,long,long,long);
#ifdef CLARICE_DRAW_DIAGNOSTICS
static int jump_legacy;
static unsigned long glyph_build_ticks;
#define JUMP_LEGACY jump_legacy
#else
#define JUMP_LEGACY 0
#endif
static int native_glyph_row(const char *s,int count){
 if(JUMP_LEGACY||!mono||font_id!=4||font_size!=9||cell_w!=6||line_h!=11||font_ascent!=9||count<1||count>85)return 0;
 for(int i=0;i<count;i++)if((unsigned char)s[i]<32||(unsigned char)s[i]>126)return 0;
 unsigned char *glyphs=prompt_pixels+STRIDE*11;
 if(!native_glyph_valid){
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
 memset(prompt_pixels,0,STRIDE*11);
 asm_ascii_cells(prompt_pixels,(const unsigned char*)s,glyphs,count,11,6);
 if(inverse){Rect r;SetRect(&r,0,0,count*6,11);InvertRect(&r);}
 return 1;
}
