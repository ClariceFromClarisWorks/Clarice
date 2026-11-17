from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];s=(r/'src/app.c').read_text();a=s.index('static int scroll_area_has_caret');b=s.index('\nstatic void input_position',a)
code='#include <assert.h>\n#include <string.h>\n#include <stdio.h>\n#define COLS 132\nstatic char last_screen[40][COLS];static int visible,columns=80;\n'+s[a:b]+r'''
int main(void){for(visible=1;visible<40;visible++)for(int row=0;row<40;row++){
 memset(last_screen,0,sizeof last_screen);last_screen[row][COLS-1]=7;
 assert(scroll_area_has_caret()==(row<visible));
 }
 puts("PASS: 1560 cursor positions/view heights; prompt and stale hidden rows cannot disable history pixel scrolling; cursors inside moved area still block it");}
'''
out=r/'work/snow-performance';(out/'scroll-guard.c').write_text(code)
subprocess.run(['cc','-std=c99','-fsanitize=address,undefined',str(out/'scroll-guard.c'),'-o',str(out/'scroll-guard')],check=True)
subprocess.run([str(out/'scroll-guard')],check=True)
