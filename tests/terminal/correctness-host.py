"""Compile the actual selection/update code with minimal Toolbox doubles."""
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];s=(r/'src/app.c').read_text();out=r/'work/terminal-correctness';out.mkdir(exist_ok=True)
def part(a,b):return s[s.index(a):s.index(b,s.index(a))]
copy=part('static int selection_line_break' if 'static int selection_line_break' in s else 'static void copy_selection','static void insert_char')
cell=part('static int cell(', 'static void update_window' if 'static void update_window' in s else 'static void handle(EventRecord *ev){')
update=part('static void update_window','static void handle(EventRecord *ev){') if 'static void update_window' in s else 'static void update_window(void){'+s.split('case updateEvt:if((WindowPtr)ev->message==win){')[1].split('}break;')[0]+'}'
head=r'''
#include <stdio.h>
#include <string.h>
#define ROWS 128
#define COLS 132
#define TEXT_LEFT 4
#define TEXT_TOP 0
static int columns=4,visible=2,cell_w=6,line_h=11,select_start,select_end;
static int busy=1,tty_reading,completing,completion_wait,searching,paging;
static long drawn_first,current_line;
static char wrapped[ROWS],text[ROWS][COLS],input[1024],last_screen[40][COLS];
static char scrap[ROWS*COLS];static int length;
static void ZeroScrap(void){length=0;}
static void PutScrap(long n,int type,const void *p){(void)type;length=n;memcpy(scrap,p,n);}
static int prompt_prefix(char *p){strcpy(p,"> ");return 2;}
typedef struct {short v,h;} Point;
static int dirty,force_draw,clipped,model,pixels,draws,win;
static void hide_caret(void){dirty=1;}
static void SetPort(int p){(void)p;}
static void sync_window_geometry(void){}
static void BeginUpdate(int p){(void)p;clipped=1;}
static void EndUpdate(int p){(void)p;clipped=0;}
static void draw(void){draws++;if(!clipped)pixels=model;dirty=force_draw=0;}
static int fails;
static void check(const char *s,int ok){printf("%s %s\n",ok?"PASS":"FAIL",s);fails+=!ok;}
'''
body=r'''
int main(void){
 current_line=2;memcpy(last_screen[0],"abcd",4);memcpy(last_screen[1],"efgh",4);memcpy(last_screen[2],"ijkl",4);
 select_start=0;select_end=8;wrapped[0]=1;copy_selection();check("history soft-wrap and hard newline",length==9&&!memcmp(scrap,"abcdefgh\r",9));
 wrapped[0]=0;copy_selection();check("real newlines preserved",length==10&&!memcmp(scrap,"abcd\refgh\r",10));
 current_line=0;busy=0;strcpy(input,"xyzw\npq");select_end=8;copy_selection();check("editable soft wrap then explicit newline",length==9&&!memcmp(scrap,"abcdefgh\r",9));
 drawn_first=1;copy_selection();check("input continuation viewport",length==9&&!memcmp(scrap,"abcd\refgh",9));drawn_first=0;
 Point p={32,28};check("one past final visible cell",cell(p)==12);p.h=32767;check("right boundary clamp",cell(p)==12);p.h=-1;p.v=-1;check("left/top clamp",cell(p)==0);
 select_start=11;select_end=12;copy_selection();check("final visible character copied",length==1&&scrap[0]=='l');
 dirty=1;model=1;pixels=0;draws=0;update_window();if(dirty)draw();check("pending pixels survive clipped update",pixels==1);check("one paint suffices for pending update",draws==1);
 dirty=force_draw=0;draws=0;update_window();check("clean update remains a single clipped paint",draws==1&&!dirty);
 return fails?1:0;
}
'''
p=out/'correctness-host.c';p.write_text(head+copy+cell+update+body)
subprocess.run(['cc','-Wno-multichar','-Wno-unused-function','-fsanitize=address,undefined',str(p),'-o',str(out/'correctness-host')],check=True)
subprocess.run([str(out/'correctness-host')],check=True)
