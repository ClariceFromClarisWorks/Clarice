from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];s=(r/'src/app.c').read_text()
a=s.index('static void input_position');b=s.index('static void pstr',a)
code=r'''
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define COLS 132
#define ROWS 128
static char text[ROWS][COLS],input[1024],search[128];
static int columns=20,visible=4,scrollback,busy,tty_reading,searching,cursor,background,paging;
static long current_line,oldest_line;
static int win=1;
static int FrontWindow(void){return 1;}
static int prompt_prefix(char *p){if(tty_reading){p[0]=0;return 0;}strcpy(p,"/ > ");return 4;}
'''+s[a:b]+r'''
int main(void){char screen[40][COLS];long first;int prompt;
 strcpy(text[0],"partial");current_line=0;strcpy(input,"xy");cursor=2;
 compose_input(screen,&first,&prompt);
 printf("partial-output: %s\n",screen[0]);
 assert(!strncmp(screen[0],"partial/ > xy",12));
 memset(text,0,sizeof text);for(int i=0;i<100;i++)snprintf(text[i],COLS,"row %d",i);
 current_line=100;strcpy(input,"012345678901234567890123456789");cursor=30;
 compose_input(screen,&first,&prompt);long live=first;
 scrollback=2;compose_input(screen,&first,&prompt);assert(first==live-2);
 for(int i=0;i<=visible;i++)assert(!screen[i][COLS-1]);
 scrollback=0;compose_input(screen,&first,&prompt);int carets=0;for(int i=0;i<=visible;i++)carets+=screen[i][COLS-1]!=0;assert(carets==1);
 
 /* A command with no trailing newline remains visible while busy, then the
    shell prompt follows it instead of replacing those bytes. */
 memset(text,0,sizeof text);oldest_line=current_line=scrollback=0;
 strcpy(text[0],"partial");input[0]=0;cursor=0;busy=1;
 compose_input(screen,&first,&prompt);assert(!strcmp(screen[0],"partial"));
 for(int i=0;i<=visible;i++)assert(!screen[i][COLS-1]);
 busy=0;compose_input(screen,&first,&prompt);
 assert(!strncmp(screen[0],"partial/ > ",11)&&screen[0][COLS-1]==12);
 /* Canonical read input follows a partial application prompt without the
    shell prefix. Exact-width text places the caret on the next whole row. */
 tty_reading=1;busy=1;strcpy(text[0],"Enter: ");strcpy(input,"abcdefghijklm");cursor=13;
 compose_input(screen,&first,&prompt);
 assert(!strncmp(screen[0],"Enter: abcdefghijklm",20));
 assert(screen[0][COLS-1]==0&&screen[1][COLS-1]==1);
 tty_reading=busy=0;
 /* A five-row input at the bottom must not detach its prefix; scrolling
    hides only the cursor, not the actual command content. */
 memset(text,0,sizeof text);current_line=100;oldest_line=0;
 for(int i=0;i<100;i++)snprintf(text[i],COLS,"row %d",i);
 memset(input,'x',80);input[80]=0;cursor=80;
 compose_input(screen,&first,&prompt);assert(first==100&&prompt==0);
 assert(!strncmp(screen[0],"/ > xxxxxxxxxxxxxxxx",20));
 assert(screen[4][COLS-1]==5);
 scrollback=1;compose_input(screen,&first,&prompt);
 assert(first==99&&prompt==1&&!strcmp(screen[0],"row 99"));
 assert(!strncmp(screen[1],"/ > xxxxxxxxxxxxxxxx",20));
 for(int i=0;i<=visible;i++)assert(!screen[i][COLS-1]);
 /* The input may exceed a viewport. Moving the caret to its beginning
    restores its prefix; moving to the end shows the final complete row. */
 scrollback=0;memset(input,'z',200);input[200]=0;cursor=200;
 compose_input(screen,&first,&prompt);assert(first==106&&prompt==-6);
 assert(screen[4][COLS-1]==5);
 cursor=0;compose_input(screen,&first,&prompt);
 assert(first==100&&prompt==0&&screen[0][COLS-1]==5);
 /* Explicit newlines and a nonzero ring origin use the same coordinates. */
 memset(text,0,sizeof text);oldest_line=198;current_line=200;
 strcpy(text[198%ROWS],"oldest");strcpy(text[199%ROWS],"previous");
 strcpy(input,"a\nb\n");cursor=4;
 compose_input(screen,&first,&prompt);
 assert(first==198&&prompt==2&&!strcmp(screen[0],"oldest"));
 assert(!strncmp(screen[2],"/ > a",5)&&!strcmp(screen[3],"b"));
 assert(screen[4][COLS-1]==1);
 background=1;compose_input(screen,&first,&prompt);
 for(int i=0;i<=visible;i++)assert(!screen[i][COLS-1]);
 puts("PASS partial output, canonical read, exact-width caret, wrapped input, scrollback, long editing, newlines, ring origin and background cursor");
}
'''
p=r/'work/viewport';p.mkdir(parents=True,exist_ok=True);(p/'compose.c').write_text(code)
subprocess.run(['cc','-std=c99','-fsanitize=address,undefined',str(p/'compose.c'),'-o',str(p/'compose')],check=True)
subprocess.run([str(p/'compose')],check=True)
# Exercise the real layout function against native-call stubs; no renderer or
# allocation policy is reimplemented here. Two regressions concern failure and
# an anchor inside the editable command's virtual continuation rows.
a=s.index('static void layout(void)');b=s.index('static void font_layout(void)',a)
pre=code[:code.index('int main(void)')]
pre=pre.replace('static int win=1;\nstatic int FrontWindow(void){return 1;}', '''
typedef struct {short top,left,bottom,right;} Rect;
struct Window {Rect portRect;};static struct Window window;
static struct Window *win=&window;
static struct Window *FrontWindow(void){return win;}
''')
layout_code=pre+r'''
#include <stdlib.h>
#include "reflow.h"
typedef char *Ptr;
#define TEXT_LEFT 4
#define TEXT_TOP 0
static int cell_w=6,line_h=12,retained=128,out_col,force_draw,dirty;
static int select_start=-1,select_end=-1,history_bar,fail_alloc;
static unsigned char wrapped[ROWS];static long page_start;
static void activity_hide(void){}
static Ptr NewPtr(long n){return fail_alloc?0:malloc(n);}
static void DisposePtr(Ptr p){free(p);}
static int scroll_limit(void){return viewport_live_first()-oldest_line;}
static void MoveControl(int a,int b,int c){(void)a;(void)b;(void)c;}
static void SizeControl(int a,int b,int c){(void)a;(void)b;(void)c;}
'''+s[a:b]+r'''
static void setup(void){
 memset(text,0,sizeof text);memset(wrapped,0,sizeof wrapped);
 current_line=100;oldest_line=0;columns=20;visible=4;
 scrollback=busy=tty_reading=searching=cursor=background=paging=0;
 input[0]=0;window.portRect=(Rect){0,0,60,140};
 for(int i=0;i<100;i++)snprintf(text[i],COLS,"row %d",i);
}
int main(int argc,char **argv){char screen[40][COLS];long before,after;int prompt;
 setup();scrollback=10;compose_input(screen,&before,&prompt);
 window.portRect.bottom=36;layout();compose_input(screen,&after,&prompt);
 assert(before==after);puts("PASS height-only viewport anchor");
 setup();
 if(argc>1&&!strcmp(argv[1],"allocation")){
  out_col=17;strcpy(text[100],"01234567890123456");
  fail_alloc=1;window.portRect.right=80;layout();
  printf("failed reflow columns=%d out_col=%d\n",columns,out_col);fflush(stdout);
  assert(columns==20&&out_col==17&&!strcmp(text[100],"01234567890123456"));
  puts("PASS failed reflow retains coherent model");
 }else{
  memset(input,'z',200);input[200]=0;cursor=200;scrollback=2;
  compose_input(screen,&before,&prompt);assert(before>current_line);
  window.portRect.right=80;layout();compose_input(screen,&after,&prompt);
  printf("editable anchor before=%ld after=%ld command=%ld\n",before,after,current_line);fflush(stdout);
  assert(after==current_line+8);
  window.portRect.right=140;layout();compose_input(screen,&after,&prompt);
  assert(after==before);
  puts("PASS input-continuation resize anchor round trip");
  setup();memset(input,'z',200);input[200]=0;cursor=200;scrollback=2;
  for(int i=39;i<200;i+=40)input[i]='\n';
  compose_input(screen,&before,&prompt);
  window.portRect.right=80;layout();compose_input(screen,&after,&prompt);
  assert(after>=current_line);
  window.portRect.right=140;layout();compose_input(screen,&after,&prompt);
  assert(after==before);
  puts("PASS newline-bearing command anchor round trip");
 }
}
'''
(p/'layout.c').write_text(layout_code)
subprocess.run(['cc','-std=c99','-fsanitize=address,undefined','-I',str(r/'src'),str(p/'layout.c'),str(r/'src/reflow.c'),'-o',str(p/'layout')],check=True)
results=[subprocess.run([str(p/'layout'),case]).returncode for case in ['allocation','input-anchor']]
if any(results):raise SystemExit('layout regressions failed: '+str(results))
