"""Compare cursor-only presentation against a full cell/attribute oracle."""
from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[2]
work=root/'work/editor-tests';work.mkdir(parents=True,exist_ok=True)
source=r'''
#include "src/ports/terminal/screen.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
static unsigned short shown[SCREEN_ROWS][SCREEN_COLS];
static unsigned short pixel(CliScreen *s,int y,int x){return (s->cells[y][x]?s->cells[y][x]:' ') | ((s->attrs[y][x]^(s->cursor&&y==s->row&&x==s->col))<<8);}
int main(void){
 CliScreen s;screen_init(&s,25,80);int oldrow=0,oldcol=0,oldshow=0;uint32_t rng=17;
 const char *ops[]={"\033[7m","\033[0m","\033[2J","\033[1;1H","\033[25;80H","\033[?25h","\033[?25l","\033[K","\r\n","hello","\033[A","\033[C","\b","\t"};
 for(int i=0;i<100000;i++){
  rng=rng*1664525+1013904223;
  for(unsigned n=0;n<1+((rng>>8)%9);n++){
   rng=rng*1664525+1013904223;const char *op=ops[(rng>>16)%14];screen_feed(&s,(const unsigned char*)op,strlen(op));
  }
  if(oldshow!=s.cursor||(s.cursor&&(oldrow!=s.row||oldcol!=s.col))){
   if(oldshow&&!(s.dirty[oldrow]&1))shown[oldrow][oldcol]^=256;
   if(s.cursor&&!(s.dirty[s.row]&1))shown[s.row][s.col]^=256;
  }
  for(int y=0;y<s.rows;y++)for(int x=0;x<s.cols;x++){
   if(s.dirty[y]&1)shown[y][x]=pixel(&s,y,x);
   assert(shown[y][x]==pixel(&s,y,x));
  }
  memset(s.dirty,0,sizeof s.dirty);oldrow=s.row;oldcol=s.col;oldshow=s.cursor;
 }
 puts("PASS: 100000 batched cursor/content/visibility updates equal full-render oracle");
}
'''
(work/'cursor.c').write_text(source)
subprocess.run(['cc','-O1','-fsanitize=address,undefined','-I',str(root),str(work/'cursor.c'),'-o',str(work/'cursor')],check=True)
subprocess.run([str(work/'cursor')],check=True)
