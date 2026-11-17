#!/usr/bin/env python3
"""Actual editor key encoder: control bytes versus native arrow events."""
from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[2]
s=(root/'src/editor-screen.inc').read_text();a=s.index('static void screen_key(');b=s.index('\nlong app_screen_read',a)
app=(root/'src/app.c').read_text()
ahead=app[app.index('if(busy){if(c==3'):app.index('\n if(c==3){char prefix',app.index('if(busy){if(c==3'))]
claim=app[app.index('static int claim_pending_screen(void){'):app.index('/* An unquoted',app.index('static int claim_pending_screen(void){'))]
code='''#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../../src/ports/terminal/screen.h"
static CliScreen screen,*editor_screen=&screen;static int failed;
static void nano_input(void){}static void input_fail(const char *s){(void)s;failed++;}
'''+s[a:b]+'''
#define LINE_CAP 1024
#define ControlKey 4096
#define shiftKey 512
#define cmdKey 256
static int busy=1,interrupted,cancelled,serial_input,serial_overflow,serial_count,serial_head,paging,activity_interactive;
static unsigned ahead_count,ahead_head;static unsigned char ahead_keys[LINE_CAP];static unsigned short ahead_mods[LINE_CAP];
static void queue(unsigned char c,unsigned short mods){
'''+ahead+'''
}
'''+claim+'''
int main(void){for(int c=0;c<256;c++){memset(&screen,0,sizeof screen);screen_key(c,1);assert(screen.count==1&&screen.keys[0]==c);}for(int c=28;c<=31;c++){memset(&screen,0,sizeof screen);screen_key(c,0);assert(screen.count==3&&screen.keys[0]==27&&screen.keys[1]=='[');}screen.count=128;screen_key('x',0);assert(failed==1&&screen.count==128);memset(&screen,0,sizeof screen);ahead_count=ahead_head=0;failed=0;
 for(int i=0;i<128;i++)queue(31,ControlKey);assert(ahead_count==128);assert(claim_pending_screen()==0);assert(screen.count==128&&!failed);for(int i=0;i<128;i++)assert(screen.keys[i]==31);
 memset(&screen,0,sizeof screen);ahead_count=ahead_head=0;failed=0;
 for(int i=0;i<129;i++)queue(31,ControlKey);assert(claim_pending_screen()==-1&&screen.count==0&&failed==1);
 memset(&screen,0,sizeof screen);ahead_count=ahead_head=0;failed=0;
 for(int i=0;i<43;i++)queue(31,0);assert(claim_pending_screen()==-1&&screen.count==0&&failed==1);

 puts("PASS: queued control modifiers and exact editor capacity; all raw control bytes distinct from arrows, bounded editor queue");}
'''
w=root/'work/ux-six';w.mkdir(parents=True,exist_ok=True);(w/'control-input.c').write_text(code)
subprocess.run(['cc','-fsanitize=address,undefined',str(w/'control-input.c'),'-o',str(w/'control-input')],check=True)
subprocess.run([str(w/'control-input')],check=True)
