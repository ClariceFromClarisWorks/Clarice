"""Production glyph extraction/composition with pixel oracle and bounded storage."""
from pathlib import Path
import subprocess
r=Path.cwd();w=r/'work/jump-perf'
head=r'''
#include <assert.h>
#include <string.h>
#include <stdio.h>
#define STRIDE 64
#define blackColor 1
#define whiteColor 0
#define DRAW_STAMP(n) ((void)0)
typedef struct {int left,top,right,bottom;} Rect;
static unsigned char prompt_pixels[4096],font[95][11];
static int native_glyph_valid,mono=1,font_id=4,font_size=9,cell_w=6,line_h=11,font_ascent=9,inverse,fore=1,back,pen_x,pen_y,draws;
static void ForeColor(int n){fore=n;}static void BackColor(int n){back=n;}
static void SetRect(Rect*r,int l,int t,int rr,int b){*r=(Rect){l,t,rr,b};}
static void bit(int x,int y,int value){assert(x>=0&&x<512&&y>=0&&y<11);unsigned char *p=prompt_pixels+y*64+x/8,m=128>>(x&7);*p=(*p&~m)|(value?m:0);}
static int getbit(int x,int y){return (prompt_pixels[y*64+x/8]>>(7-(x&7)))&1;}
static void EraseRect(Rect*r){for(int y=r->top;y<r->bottom;y++)for(int x=r->left;x<r->right;x++)bit(x,y,back);}
static void InvertRect(Rect*r){for(int y=r->top;y<r->bottom;y++)for(int x=r->left;x<r->right;x++)bit(x,y,!getbit(x,y));}
static void MoveTo(int x,int y){pen_x=x;pen_y=y;}
static void DrawText(const char*s,int off,int n){draws++;for(int c=0;c<n;c++)for(int y=0;y<11;y++)for(int x=0;x<6;x++)bit(pen_x+c*6+x,pen_y-9+y,(font[s[off+c]-32][y]>>(7-x)&1)?fore:back);}
void asm_ascii_cells(unsigned char*d,const unsigned char*s,const unsigned char*g,long n,long h,long width){
 for(int c=0;c<n;c++)for(int y=0;y<h;y++)for(int x=0;x<width;x++){int b=c*width+x;unsigned char mask=128>>(b&7);if(c<(n/4)*4)d[y*64+b/8]&=~mask;if(g[(s[c]-32)*32+y]&(128>>x))d[y*64+b/8]|=mask;}
}
'''
tail=r'''
int main(void){unsigned seed=68000;for(int c=0;c<95;c++)for(int y=0;y<11;y++){seed=seed*1664525+1013904223; font[c][y]=c?(seed>>24)&0xfc:0;}
 for(int rebuild=0;rebuild<3;rebuild++){
  memset(prompt_pixels,0xa5,sizeof prompt_pixels);native_glyph_valid=0;draws=0;
  for(inverse=0;inverse<2;inverse++)for(int n=1;n<=85;n++){
   memset(prompt_pixels,0xa5,704);
   char s[85];for(int i=0;i<n;i++)s[i]=32+(n*11+i*13)%95;
   assert(native_glyph_row(s,n)==1);assert(draws==2);
   for(int y=0;y<11;y++)for(int x=0;x<n*6;x++){int want=((font[s[x/6]-32][y]>>(7-x%6))&1)^inverse;assert(getbit(x,y)==want);}
   for(int c=0;c<95;c++)assert(!memcmp(prompt_pixels+704+c*32,font[c],11));
   for(int i=3744;i<4096;i++)assert(prompt_pixels[i]==0xa5);
  }
 }
 unsigned char before[4096];memcpy(before,prompt_pixels,4096);char bad[2]={127,0};assert(!native_glyph_row(bad,1));bad[0]=(char)200;assert(!native_glyph_row(bad,1));bad[0]=31;assert(!native_glyph_row(bad,1));font_id=3;assert(!native_glyph_row("ABC",3));font_id=4;font_size=12;assert(!native_glyph_row("ABC",3));font_size=9;assert(!native_glyph_row("A",0));assert(!native_glyph_row("A",86));assert(!memcmp(before,prompt_pixels,4096));
 puts("PASS: 510 production glyph-row cases, all 95 ASCII glyphs, widths 1..85, both themes, two-draw cold preparation, cache reuse/rebuild, tail guards and unchanged unsupported fallbacks");
}
'''
(w/'native-glyphs.c').write_text(head+(r/'src/native-glyphs.inc').read_text()+tail)
subprocess.run(['cc','-std=c99','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(w/'native-glyphs.c'),'-o',str(w/'native-glyphs')],check=True)
subprocess.run([str(w/'native-glyphs')],check=True)
