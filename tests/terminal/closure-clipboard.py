"""Presented characters and line endings must survive pending output/reflow."""
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];s=(r/'src/app.c').read_text();out=r/'work/terminal-closure';out.mkdir(exist_ok=True)
head=r'''
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define COLS 132
#define ROWS 128
static char text[ROWS][COLS],input[1024],search[128],wrapped[ROWS],last_screen[40][COLS];
static int columns=20,visible=4,scrollback,busy,completing,completion_wait,tty_reading,searching,cursor,background,paging;
static long current_line,oldest_line;
static int win=1;
static int FrontWindow(void){return 1;}
static int prompt_prefix(char *p){if(tty_reading){p[0]=0;return 0;}strcpy(p,"/ > ");return 4;}
static long drawn_first;static int select_start,select_end,length;static char scrap[20000];
static void ZeroScrap(void){length=0;}
static void PutScrap(long n,int t,const void*p){(void)t;length=n;memcpy(scrap,p,n);}
'''
start=s.index('/* History records real versus visual') if '/* History records real versus visual' in s else s.index('static void copy_selection')
copy=s[start:s.index('static void insert_char',start)]
code=head+s[s.index('static void input_position'):s.index('static void pstr')]+copy+r'''
static void paint(void){int prompt;compose_input(last_screen,&drawn_first,&prompt);}
int main(void){
 columns=4;visible=2;busy=1;current_line=2;strcpy(text[0],"abcd");strcpy(text[1],"efgh");wrapped[0]=1;
 paint();select_start=0;select_end=8;copy_selection();assert(length==9&&!memcmp(scrap,"abcdefgh\r",9));
 /* Reused history slots must not reinterpret the painted frame. */
 wrapped[0]=0;wrapped[1]=1;current_line=130;oldest_line=3;copy_selection();assert(length==9&&!memcmp(scrap,"abcdefgh\r",9));
 current_line=oldest_line=0;memset(text,0,sizeof text);busy=0;tty_reading=1;strcpy(input,"abcdEF\nxy");cursor=9;paint();
 select_start=0;select_end=8;strcpy(input,"ab\ncd");busy=1;tty_reading=0;copy_selection();assert(length==9&&!memcmp(scrap,"abcdEF  \r",9));
 puts("PASS presented soft/hard boundaries survive pending history output, ring reuse and changed editable input");
 return 0;
}
'''
p=out/'clipboard-host.c';p.write_text(code)
subprocess.run(['cc','-Wno-multichar','-fsanitize=address,undefined',str(p),'-o',str(p.with_suffix(''))],check=True)
subprocess.run([str(p.with_suffix(''))],check=True)
