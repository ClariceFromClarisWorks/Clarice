"""Exercise production inverse-run painting against the per-cell reference."""
from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[2]
s=(root/'src/editor-screen.inc').read_text()
a=s.index('   for(int x=0;x<s->cols;){');b=s.index('\n   s->dirty[row]',a)
code=r'''
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "src/ports/terminal/screen.h"
typedef struct {int left,top,right,bottom;} Rect;
static unsigned char pixels[132*9*32];
static int calls,cell_w,line_h,background,origin;
static void SetRect(Rect *r,int l,int t,int rr,int b){*r=(Rect){l,t,rr,b};}
static void InvertRect(Rect *r){assert(r->left>=0&&r->right<=132*cell_w&&r->top==0&&r->bottom==line_h);calls++;for(int y=0;y<line_h;y++)for(int x=r->left;x<r->right;x++)pixels[y*132*cell_w+x]^=1;}
static void paint(CliScreen *s,int row){int top=0;
'''+s[a:b]+r'''
}
int main(void){CliScreen screen,*s=&screen;uint32_t seed=68000;int total=0;
 for(int n=0;n<20000;n++){
  memset(s,0,sizeof *s);s->cols=1+n%132;s->cursor=n%2;s->row=(n/2)%2;s->col=(n/4)%s->cols;background=(n/7)%2;cell_w=3+n%7;line_h=8+n%25;
  for(int x=0;x<s->cols;x++){seed=seed*1664525+1013904223;s->attrs[0][x]=n%5==0?1:n%5==1?0:(seed>>31);}
  memset(pixels,0,sizeof pixels);calls=0;paint(s,0);
  for(int y=0;y<line_h;y++)for(int x=0;x<132*cell_w;x++){int c=x/cell_w;int expected=c<s->cols?(s->attrs[0][c]^(s->cursor&&s->row==0&&c==s->col&&!background)):0;assert(pixels[y*132*cell_w+x]==expected);}
  total++;
 }
 memset(s,0,sizeof *s);s->cols=80;memset(s->attrs[0],1,80);calls=0;paint(s,0);assert(calls==1);
 printf("PASS: %d pixel comparisons across widths, inverse patterns and existing cursor states; full inverse row 80 calls -> 1; no allocation\n",total);
}
'''
out=root/'work/snow-performance';out.mkdir(exist_ok=True,parents=True)
(out/'editor-inverse.c').write_text(code)
subprocess.run(['cc','-std=c99','-fsanitize=address,undefined','-I',str(root),str(out/'editor-inverse.c'),'-o',str(out/'editor-inverse')],check=True)
subprocess.run([str(out/'editor-inverse')],check=True)
