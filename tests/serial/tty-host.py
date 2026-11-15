#!/usr/bin/env python3
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];s=(r/'src/app.c').read_text();f=s[s.index('void term_write('):s.index('void term_puts(')]
head='''#include <string.h>
#include <assert.h>
#include <stdio.h>
#define ROWS 128
static char text[ROWS][132];static int current_line,out_col,columns=80,last_cr,serial_input,dirty,test_ref=-1,activity_output_tick;
static void activity_hide(void){} static int TickCount(void){return 0;}
static void FSWrite(int r,long *n,const char *s){(void)r;(void)n;(void)s;}
static void newline(void){current_line++;out_col=0;memset(text[current_line],0,132);}static void soft_newline(void){newline();}
'''
tail='''int main(void){
 serial_input=2;term_write("MACBOOK> echo bad",17);term_write("\\b \\b\\b \\b\\b \\bgood\\r\\ngood\\r\\nMACBOOK> ",30);
 assert(!strncmp(text[0],"MACBOOK> echo good",18));assert(!strncmp(text[1],"good",4));assert(!strncmp(text[2],"MACBOOK> ",9));
 term_write("abcdef\\rxy",9);assert(out_col==2);assert(text[2][0]=='x'&&text[2][1]=='y');
 serial_input=0;last_cr=0;current_line=out_col=0;memset(text,0,sizeof text);term_write("old\\r\\nnext",9);assert(current_line==1&&!strncmp(text[0],"old",3)&&!strncmp(text[1],"next",4));
 puts("PASS: remote CR/LF/BS erase and overwrite, legacy CR/LF unchanged");}
'''
p=r/'work/serial-2/tty-host.c';p.write_text(head+f+tail);subprocess.run(['cc','-fsanitize=address,undefined',str(p),'-o',str(p.with_suffix(''))],check=True);subprocess.run([str(p.with_suffix(''))],check=True)
