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
static char text[ROWS][COLS],input[1024],search[128],wrapped[ROWS];
static int columns=20,visible=4,scrollback,busy,completing,completion_wait,tty_reading,searching,cursor,background,paging;
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
 {char ordinary[40][COLS];memcpy(ordinary,screen,sizeof ordinary);busy=completing=1;compose_input(screen,&first,&prompt);assert(!memcmp(ordinary,screen,sizeof ordinary));completion_wait=1;compose_input(screen,&first,&prompt);for(int i=0;i<=visible;i++)assert(!screen[i][COLS-1]);busy=completing=completion_wait=0;}
 background=1;compose_input(screen,&first,&prompt);
 for(int i=0;i<=visible;i++)assert(!screen[i][COLS-1]);
 puts("PASS partial output, canonical read, exact-width caret, wrapped input, scrollback, long editing, newlines, ring origin and background cursor");
}
'''
p=r/'work/viewport';p.mkdir(parents=True,exist_ok=True);(p/'compose.c').write_text(code)
subprocess.run(['cc','-std=c99','-fsanitize=address,undefined',str(p/'compose.c'),'-o',str(p/'compose')],check=True)
subprocess.run([str(p/'compose')],check=True)
