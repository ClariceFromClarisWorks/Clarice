#!/usr/bin/env python3
"""Exercise the actual compositor with host QuickDraw-independent stubs."""
from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[2]
source=(root/'src/app.c').read_text()
body=source[source.index('static void compose_input'):source.index('static void pstr')]
prefix='#include <stdio.h>\n#include <string.h>\n#include <assert.h>\n#define COLS 132\n#define ROWS 128\nstatic int columns,visible=24,busy,tty_reading,searching,background,scrollback,cursor,paging;\nstatic long current_line,oldest_line;\nstatic char input[1024],text[128][132],search[128];\nstatic int win=1;\nstatic int FrontWindow(void){return 1;}\nstatic int prompt_prefix(char *s){strcpy(s,"> ");return 2;}\n'
tail='int main(void){char screen[40][132];long first;int prompt;\n for(columns=3;columns<=130;columns++)for(int n=0;n<1024;n+=7){memset(input,\'x\',n);input[n]=0;for(cursor=0;cursor<=n;cursor+=3){compose_input(screen,&first,&prompt);int found=0;for(int r=0;r<40;r++)if(screen[r][131]){found++;assert((unsigned char)screen[r][131]-1==(cursor+2)%columns);assert(r<=visible);}assert(found==1);}}\n puts("PASS: wrapped input and cursor bounds across widths 3..130 and 0..1023 input bytes");}\n'
target=root/'work/input-layout-test.c'
target.write_text(prefix+body+tail)
subprocess.run(['cc','-fsanitize=address,undefined',str(target),'-o',str(root/'work/input-layout-test')],check=True)
subprocess.run([str(root/'work/input-layout-test')],check=True)

# Compile and exercise the interactive trailing-backslash recognizer as well.
body=source[source.index('static int continues_line'):source.index('static void execute(void){')]
checks='int main(void){assert(continues_line("echo a\\\\"));assert(!continues_line("echo a\\\\\\\\"));assert(!continues_line("echo \'a\\\\"));assert(continues_line("echo \\"a\\\\"));assert(!continues_line("echo a\\\\\\nb"));assert(continues_line("echo a\\\\\\nb\\\\"));return 0;}\n'
target=root/'work/continuation-test.c'
target.write_text('#include <assert.h>\n'+body+checks)
subprocess.run(['cc','-fsanitize=address,undefined',str(target),'-o',str(root/'work/continuation-test')],check=True)
subprocess.run([str(root/'work/continuation-test')],check=True)
print('PASS: escaped, quoted and chained line continuation cases')
