"""Actual paste/bounds helpers with ASan/UBSan and a bounded Scrap Manager mock."""
from pathlib import Path
import subprocess,tempfile
s=Path('src/app.c').read_text()
def fn(name):
 a=s.index('static void '+name+'(')
 while s[s.index(')',a)+1]==';': a=s.index('static void '+name+'(',a+1)
 b=s.index('{',a);n=1;e=b+1
 while n:n+=(s[e]=='{')-(s[e]=='}');e+=1
 return s[a:e]
head=r'''
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define LINE_CAP 1024
#define TEXT 0
static char input[LINE_CAP],continued[LINE_CAP];
static int input_blocked,cursor,continuing,busy,tty_reading,cancelled,dirty,shell_status;
static unsigned ahead_head,ahead_count;
static int errors,allocation_fail,scrap_fail;
static char scrap[4096];static long scrap_len;
typedef char **Handle;
static Handle NewHandle(int n){(void)n;if(allocation_fail)return 0;Handle p=malloc(sizeof(char*));*p=0;return p;}
static void DisposeHandle(Handle h){free(*h);free(h);}
static long GetScrap(Handle h,long type,long *off){(void)type;(void)off;if(scrap_fail)return -1;*h=malloc(scrap_len+1);memcpy(*h,scrap,scrap_len);return scrap_len;}
static void HLock(Handle h){(void)h;}static void HUnlock(Handle h){(void)h;}
static void term_puts(const char *p){if(strstr(p,"Input rejected"))errors++;}
static void reset(void){input_blocked=cursor=continuing=busy=tty_reading=cancelled=0;ahead_count=0;input[0]=continued[0]=0;}
'''
main=r'''
int main(void){
 for(int old=0;old<1024;old+=17)for(int n=0;n<1100;n+=13){reset();memset(input,'x',old);input[old]=0;cursor=old/2;memset(scrap,'y',n);scrap_len=n;paste();if(old+n>1023){assert(input_blocked&&!input[0]);}else{assert(!input_blocked&&strlen(input)==old+n);assert(!memcmp(input+old/2,scrap,n));assert(cursor==old/2+n);}}
 reset();strcpy(scrap,"a\r\nb\rc\n");scrap_len=7;paste();assert(!strcmp(input,"a\nb\nc\n"));
 reset();scrap[0]='a';scrap[1]=0;scrap[2]='b';scrap_len=3;paste();assert(input_blocked&&!input[0]);
 reset();allocation_fail=1;paste();assert(input_blocked);allocation_fail=0;
 reset();scrap_fail=1;paste();assert(input_blocked);scrap_fail=0;
 reset();for(int i=0;i<1023;i++)insert_char('a');assert(!input_blocked);insert_char('b');assert(input_blocked&&!input[0]);int was=errors;insert_char('c');assert(errors==was&&!input[0]);
 puts("PASS: bounded atomic insertion, CR/LF normalization, unsupported controls, allocation/read failure, typed overflow latch");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'test.c';exe=Path(d)/'test';p.write_text(head+fn('input_fail')+fn('insert_char')+fn('paste')+main)
 subprocess.run(['cc','-std=c99','-Wno-multichar','-fsanitize=address,undefined',str(p),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
