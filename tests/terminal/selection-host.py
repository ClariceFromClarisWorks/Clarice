#!/usr/bin/env python3
"""Exercise production selection delta rectangles against full selection masks."""
from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[2]
s=(root/'src/app.c').read_text()
a=s.index('static void selection_extend('); b=s.index('\nstatic void selection_begin(',a)
code=r'''
#include <assert.h>
#include <string.h>
#include <stdio.h>
typedef struct { int left,top,right,bottom; } Rect;
static int columns,visible,cell_w=1,line_h=1,select_start,select_end,render_mode;
static int frame,window,port;static int *win=&window,*frame_pixels;
static struct { int ignored; } frame_port;
#define TEXT_LEFT 3
#define TEXT_TOP 2
static unsigned char live[40][132],cache[40][132];
static void SetPort(void *p){port=p==win?0:1;}
static void SetRect(Rect*r,int l,int t,int rr,int b){*r=(Rect){l,t,rr,b};}
static void OffsetRect(Rect*r,int h,int v){r->left+=h;r->right+=h;r->top+=v;r->bottom+=v;}
static void HideCursor(void){} static void ShowCursor(void){}
static void InvertRect(Rect*r){int l=r->left-(port?0:TEXT_LEFT),t=r->top-(port?0:TEXT_TOP),rr=r->right-(port?0:TEXT_LEFT),b=r->bottom-(port?0:TEXT_TOP); assert(l>=0&&rr<=columns&&t>=0&&b<=visible);for(int y=t;y<b;y++)for(int x=l;x<rr;x++)(port?cache:live)[y][x]^=1;}
'''
tail=r'''
int main(void){for(columns=1;columns<=130;columns+=3)for(visible=1;visible<=37;visible+=4)for(render_mode=0;render_mode<=2;render_mode++){
 frame_pixels=render_mode?&frame:0;memset(live,0,sizeof live);memset(cache,0,sizeof cache);select_start=select_end=columns*visible/2;
 unsigned seed=23;for(int i=0;i<200;i++){seed=seed*1664525u+1013904223u;int end=seed%(columns*visible);selection_extend(end);int lo=end<select_start?end:select_start,hi=end>select_start?end:select_start;for(int y=0;y<visible;y++)for(int x=0;x<columns;x++){int expected=y*columns+x>=lo&&y*columns+x<hi;assert(live[y][x]==expected);if(render_mode)assert(cache[y][x]==expected);}selection_extend(end);}
 selection_clear();for(int y=0;y<visible;y++)for(int x=0;x<columns;x++){assert(!live[y][x]);if(render_mode)assert(!cache[y][x]);}
 }puts("PASS selection delta: forward/reverse/anchor crossing/stationary, 1–130 columns, 1–37 rows, native and cache");}
'''
w=root/'work/qol3a';w.mkdir(parents=True,exist_ok=True);(w/'selection.c').write_text(code+s[a:b]+tail)
subprocess.run(['cc','-std=c99','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fsanitize=address,undefined',str(w/'selection.c'),'-o',str(w/'selection-host')],check=True)
subprocess.run([str(w/'selection-host')],check=True)
