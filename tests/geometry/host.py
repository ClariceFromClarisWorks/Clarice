"""Exercise production geometry/layout with native Window Manager stubs."""
from pathlib import Path
import subprocess,tempfile
s=Path('src/app.c').read_text()
viewport=s[s.index('static void input_position'):s.index('static void pstr',s.index('static void input_position'))]
layout=s[s.index('/* Native structure/content bounds'):s.index('static void font_layout(void){')]
pre=r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reflow.h"
#define ROWS 128
#define COLS 132
#define HEIGHT 336
#define TEXT_LEFT 4
#define TEXT_TOP 0
typedef struct {short top,left,bottom,right;} Rect;
typedef struct {short v,h;} Point;
typedef struct {Rect rgnBBox;} Region;
typedef struct {Rect portRect;Region **strucRgn;} Window;
typedef Window *WindowPeek;
static Region region,*regionp=&region;static Window window, *win=&window;
static struct {struct {Rect bounds;} screenBits;} qd;
static int originx=4,originy=40,sizes,moves,allocations,fail_alloc,beeps;
static short layout_width,layout_height,cell_w=6,line_h=11;
static int filled,saves,invalidations;static void *editor_screen;
static int columns=66,visible=23,scrollback,busy,completing,completion_wait,tty_reading,searching,cursor,background,paging;
static int retained=128,out_col,page_start,select_start=-1,select_end=-1,dirty,force_draw,history_bar=1;
static long current_line,oldest_line;
static char text[ROWS][COLS],input[1024],search[128];static unsigned char wrapped[ROWS];
static void region_update(void){region.rgnBBox=(Rect){originy-18,originx-2,originy+win->portRect.bottom+2,originx+win->portRect.right+2};}
static void SetPort(Window *w){(void)w;}
static void LocalToGlobal(Point *p){p->h+=originx;p->v+=originy;}
static int menu_bar_height(void){return 20;}
static void SizeWindow(Window *w,int x,int y,int update){(void)update;w->portRect.right=x;w->portRect.bottom=y;sizes++;region_update();}
static void MoveWindow(Window *w,int x,int y,int front){(void)w;(void)front;originx=x;originy=y;moves++;region_update();}
static unsigned long last_yield=1,clock_tick=10;
static int shell_session_ui_begin(void){return 0;}
static void shell_session_ui_end(int n){(void)n;}
static void draw(void){clock_tick+=20;}
static void activity_hide(void){}
static void save_preferences(void){saves++;}
static void InvalRect(Rect *r){(void)r;invalidations++;}
static void *NewPtr(size_t n){if(fail_alloc)return 0;allocations++;return malloc(n);}
typedef char *Ptr;
static void DisposePtr(Ptr p){allocations--;free(p);}
static void SysBeep(int n){(void)n;beeps++;}
static void MoveControl(int c,int x,int y){(void)c;(void)x;(void)y;}
static void SizeControl(int c,int x,int y){(void)c;(void)x;(void)y;}
static Window *FrontWindow(void){return win;}
static int prompt_prefix(char *p){strcpy(p,"/ > ");return 4;}
static int scroll_limit(void){return current_line+128;}
'''
main=r'''
int main(void){
 win->strucRgn=&regionp;qd.screenBits.bounds=(Rect){0,0,342,512};SizeWindow(win,420,264,1);assert(layout());
 assert(columns==66&&visible+1==24&&allocations==0);
 for(int cw=4;cw<=24;cw++)for(int lh=6;lh<=32;lh++)for(int n=1;n<1000;n+=71){cell_w=cw;line_h=lh;grid_window(n,n,cw,lh);assert(layout());assert(columns*cw+20<=win->portRect.right);assert((visible+1)*lh<=win->portRect.bottom);assert(region.rgnBBox.right<=510&&region.rgnBBox.bottom<=340&&region.rgnBBox.top>=22);assert(!allocations);}
 cell_w=6;line_h=11;grid_window(66,24,6,11);assert(layout());
 strcpy(text[0],"abcdefghij");wrapped[0]=1;strcpy(text[1],"klmnop");current_line=1;out_col=6;
 memset(input,'q',900);input[900]=0;cursor=450;char saved[1024];strcpy(saved,input);
 grid_window(40,15,6,11);assert(layout());assert(!strcmp(input,saved)&&cursor==450);assert(!strcmp(text[0],"abcdefghijklmnop"));
 char screen[40][COLS];long first;int prompt;compose_input(screen,&first,&prompt);int carets=0;for(int i=0;i<=visible;i++)carets+=screen[i][COLS-1]!=0;assert(carets==1);
 int w=layout_width,h=layout_height,c=columns,v=visible;char savedline[132];strcpy(savedline,text[0]);fail_alloc=1;grid_window(60,20,6,11);assert(!layout());assert(win->portRect.right==w&&win->portRect.bottom==h&&columns==c&&visible==v&&!strcmp(savedline,text[0])&&!strcmp(saved,input)&&!allocations);
 fail_alloc=0;grid_window(60,20,6,11);assert(layout()&&columns==60&&visible+1==20&&!allocations);
 int oldsizes=sizes,oldalloc=allocations;sync_window_geometry();assert(sizes==oldsizes&&allocations==oldalloc&&!saves);
 SizeWindow(win,300,155,1);sync_window_geometry();assert(columns==46&&visible+1==14&&saves==1&&invalidations==1);
 editor_screen=win;SizeWindow(win,400,200,1);sync_window_geometry();assert(win->portRect.right==300&&win->portRect.bottom==155&&columns==46&&visible+1==14&&saves==1);editor_screen=0;
 dirty=1;term_present();assert(clock_tick==30&&last_yield==1);
 puts("PASS production geometry: 8,505 metric/bounds combinations, actual rows, long-input caret/state, reflow, allocation failure rollback/retry and zero retained allocations");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'geometry.c';p.write_text(pre+viewport+layout+s[s.index('static void sync_window_geometry'):s.index('/* Repeated metadata rows')]+s[s.index('void term_present(void)'):s.index('\n',s.index('void term_present(void)'))]+main)
 subprocess.run(['cc','-fsanitize=address,undefined','-Isrc',str(p),'src/reflow.c','-o',str(p.with_suffix(''))],check=True)
 subprocess.run([str(p.with_suffix(''))],check=True)
