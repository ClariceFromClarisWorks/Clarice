from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];out=r/'work/snow-performance'
head=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "src/ports/terminal/screen.h"
typedef struct {int left,top,right,bottom;} Rect;
typedef struct {unsigned char pixels[512*1280];int fore,back,rowBytes;Rect bounds;} Bitmap;
typedef struct {Bitmap portBits;} Port;
static Port prompt_port,window,*win=&window,*port;
static unsigned char reference[512*1280];
static CliScreen model,*editor_screen=&model;
static int font_id,font_size,inverse,mono,force_draw,dirty,line_h,cell_w,font_ascent,background,cx,cy,copies;
static unsigned long last_paint;
#define TEXT_LEFT 3
#define TEXT_TOP 2
#define whiteColor 0
#define blackColor 1
#define srcCopy 0
static void SetPort(Port *p){port=p;}
static void TextFont(int n){(void)n;}static void TextSize(int n){(void)n;}static void TextFace(int n){(void)n;}static void TextMode(int n){(void)n;}
static void ForeColor(int n){port->portBits.fore=n;}static void BackColor(int n){port->portBits.back=n;}
static void SetRect(Rect *r,int l,int t,int rr,int b){*r=(Rect){l,t,rr,b};}
static void OffsetRect(Rect *r,int x,int y){r->left+=x;r->right+=x;r->top+=y;r->bottom+=y;}
static void EraseRect(Rect *r){assert(r->left>=0&&r->right<=512&&r->top>=0&&r->bottom<=64);for(int y=r->top;y<r->bottom;y++)for(int x=r->left;x<r->right;x++)port->portBits.pixels[y*512+x]=port->portBits.back;}
static void InvertRect(Rect *r){assert(r->left>=0&&r->right<=512&&r->top>=0&&r->bottom<=64);for(int y=r->top;y<r->bottom;y++)for(int x=r->left;x<r->right;x++)port->portBits.pixels[y*512+x]^=1;}
static void MoveTo(int x,int y){cx=x;cy=y;}
static void DrawChar(unsigned char c){for(int y=0;y<line_h;y++)for(int x=0;x<cell_w;x++){int bit=c!=' '&&((c>>(x%8))^(y%3))&1;port->portBits.pixels[(cy-font_ascent+y)*512+cx+x]=bit?port->portBits.fore:port->portBits.back;}cx+=cell_w;}
static void DrawText(const char *s,int at,int n){while(n--)DrawChar(s[at++]);}
static void CopyBits(Bitmap *a,Bitmap *b,Rect *src,Rect *dst,int mode,void *mask){(void)mode;(void)mask;copies++;assert(src->left>=0&&src->right<=512&&src->bottom<=64);assert(src->right-src->left==dst->right-dst->left&&src->bottom-src->top==dst->bottom-dst->top);for(int y=0;y<src->bottom-src->top;y++)for(int x=0;x<src->right-src->left;x++){int xx=dst->left+x,yy=dst->top+y;/* Include a clipped window case. */if(xx>=10&&xx<490&&yy>=5&&yy<1270)b->pixels[yy*512+xx]=a->pixels[(src->top+y)*512+src->left+x];}}
static unsigned long TickCount(void){return 0;}static void nano_painted(void){}
'''
s=(r/'src/editor-screen.inc').read_text();code=head+(r/'tests/terminal/editor-reference.inc').read_text()+s[s.index('static void draw_editor(void)'):]+r'''
int main(void){uint32_t seed=1;for(int n=0;n<300;n++){
 window.portBits.rowBytes=n%3?64:128;window.portBits.bounds.left=-(n%16);prompt_port.portBits.bounds.right=512;
 cell_w=3+n%6;line_h=8+n%25;font_ascent=line_h-3;mono=n%2;inverse=(n/2)%2;background=(n/4)%2;
 screen_init(&model,1+n%39,(n%7?480:508)/cell_w<132?(n%7?480:508)/cell_w:132);model.cursor=n%3!=0;model.row=n%model.rows;model.col=n%model.cols;
 for(int y=0;y<model.rows;y++){model.dirty[y]=(n%5==0)||(y%3!=0);for(int x=0;x<model.cols;x++){seed=seed*1664525+1013904223;model.cells[y][x]=(n%11==0||x>model.cols-(n%model.cols))?32:32+(seed>>24)%95;model.attrs[y][x]=(seed>>16)%2;}}
 CliScreen saved=model;force_draw=n%4==0;memset(window.portBits.pixels,0x55,sizeof reference);draw_reference();memcpy(reference,window.portBits.pixels,sizeof reference);
 memset(prompt_port.portBits.pixels,0xa5,sizeof prompt_port.portBits.pixels);model=saved;force_draw=n%4==0;memset(window.portBits.pixels,0x55,sizeof reference);draw_editor();assert(!memcmp(reference,window.portBits.pixels,sizeof reference));
 }
 line_h=11;cell_w=6;mono=1;screen_init(&model,25,80);force_draw=1;copies=0;draw_editor();assert(copies==5);
 puts("PASS: 300 complete production-renderer comparisons, mixed dirty rows, font metrics, attributes, cursor, colours and clipping; full screen copies 25 -> 5; staging bounds checked");
}
'''
(out/'editor-render.c').write_text(code)
subprocess.run(['cc','-std=c99','-fsanitize=address,undefined','-I',str(r),str(out/'editor-render.c'),'-o',str(out/'editor-render')],check=True)
subprocess.run([str(out/'editor-render')],check=True)
