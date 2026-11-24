"""Production word navigation, dispatch guard and wrapped-cursor composition."""
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];s=(r/'src/app.c').read_text()
helpers=s[s.index('static int word_key('):s.index('static void key(unsigned char c,unsigned short mods){')]
cases=s[s.index('case 2:case 28:'):s.index('\n case 16:case 30:')]
gate=s[s.index(' if(!busy&&!editor_screen&&!tty_reading&&!searching&&!paging&&word_key'):s.index(' unsigned short mods=ev->modifiers;int explicit_control=')]
compose=s[s.index('static void input_position'):s.index('static void pstr',s.index('static void input_position'))]
code=r'''
#include <assert.h>
#include <string.h>
#include <stdio.h>
#define cmdKey 256
#define shiftKey 512
#define optionKey 2048
#define ControlKey 4096
#define COLS 132
#define ROWS 128
static char text[ROWS][COLS],input[1024],search[128];
static int columns=20,visible=4,scrollback,busy,completing,completion_wait,tty_reading,searching,cursor,background,paging,editor_screen;
static long current_line,oldest_line;
static int win=1,called;
static int FrontWindow(void){return 1;}
static int prompt_prefix(char *p){strcpy(p,"/ > ");return 4;}
'''+helpers+compose+r'''
static void key(unsigned char c,unsigned short mods){int n=strlen(input);called++;switch(c){
'''+cases+r'''
}}
static int dispatch(unsigned char c,unsigned short mods){struct {unsigned short modifiers;} event={mods},*ev=&event;called=0;do{
'''+gate+r'''
}while(0);return called;}
static int white(unsigned char c){return c==32||(c>=9&&c<=13);}
static int reference(const char *s,int p,int right){int n=strlen(s);
 if(right){int q=p;for(;q<n&&!white(s[q]);q++){}for(;q<n&&white(s[q]);q++){}return q;}
 int q=p-1;for(;q>=0&&white(s[q]);q--){}for(;q>=0&&!white(s[q]);q--){}return q+1;
}
int main(void){const char *samples[]={"","word","  word  ","a  b\t\tc ","a\r\n\v\fb","echo 'two words' /mnt/Macintosh\\ Exchange/file","\t \t",0};int checks=0;
 for(int t=0;samples[t];t++){strcpy(input,samples[t]);char original[1024];strcpy(original,input);int n=strlen(input);
 for(int p=0;p<=n;p++)for(int right=0;right<2;right++){
 cursor=p;key(right?29:28,cmdKey);assert(cursor==reference(input,p,right));assert(!strcmp(original,input));checks++;
 cursor=p;key(right?29:28,0);assert(cursor==(right?(p<n?p+1:p):(p?p-1:0)));checks++;
 }
 cursor=n;for(int i=0;i<n+5;i++)key(28,cmdKey);assert(cursor==0);
 for(int i=0;i<n+5;i++)key(29,cmdKey);assert(cursor==n);
 }
 strcpy(input,"one two");for(unsigned mods=0;mods<8192;mods++){
 cursor=7;assert(dispatch(28,mods)==((mods&cmdKey)&&!(mods&(ControlKey|optionKey))));
 assert(!dispatch('c',mods)&&!dispatch('v',mods)&&!dispatch('q',mods)&&!dispatch('.',mods));
 }
 int *owners[]={&busy,&editor_screen,&tty_reading,&searching,&paging};
 for(int i=0;i<5;i++){*owners[i]=1;assert(!dispatch(28,cmdKey));*owners[i]=0;}
 /* Same recalled text, moved then edited; no quoting or pathname rewriting. */
 strcpy(input,"echo recalled value");cursor=strlen(input);key(28,cmdKey);assert(cursor==14);
 memmove(input+cursor+1,input+cursor,strlen(input+cursor)+1);input[cursor]='X';assert(!strcmp(input,"echo recalled Xvalue"));
 for(int i=0;i<1023;i++)input[i]=(i%17==16)?' ':'x';input[1023]=0;
 char before[40][COLS],after[40][COLS];long f,g;int p,q;
 columns=60;visible=30;cursor=1023;compose_input(before,&f,&p);
 for(int i=0;i<140;i++){
 key(i<70?28:29,cmdKey);compose_input(after,&g,&q);int caret=0;
 for(int row=0;row<=visible;row++){caret+=after[row][COLS-1]!=0;assert(!memcmp(before[row],after[row],COLS-1));}
 assert(caret==1&&f==g&&p==q);
 int row,col;input_position(4,cursor,&row,&col);assert(after[row][COLS-1]==col+1);
 }
 assert(cursor==1023);puts("PASS production word/plain movement, whitespace/quotes/paths, boundaries, 8192 modifier combinations, ownership guard, recalled editing and wrapped cursor/text invariance; allocation-free");
}
'''
w=r/'work/word-navigation';w.mkdir(parents=True,exist_ok=True);(w/'host.c').write_text(code)
subprocess.run(['cc','-O1','-fsanitize=address,undefined',str(w/'host.c'),'-o',str(w/'host')],check=True)
subprocess.run([str(w/'host')],check=True)
