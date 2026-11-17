"""Check rendered-state preservation and conservative dirty rows against baseline."""
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];out=r/'work/snow-performance'
old=(r/'tests/terminal/screen-reference.h').read_text().replace('CLARICE_SCREEN_H','OLD_CLARICE_SCREEN_H').replace('CliScreen','OldScreen').replace('screen_','old_screen_')
code=old+'\n#include "src/ports/terminal/screen.h"\n'+r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
static int changed(CliScreen *a,CliScreen *b,int row){
 if(memcmp(a->cells[row],b->cells[row],a->cols)||memcmp(a->attrs[row],b->attrs[row],a->cols))return 1;
 for(int x=0;x<a->cols;x++)if((a->cursor&&a->row==row&&a->col==x)!=(b->cursor&&b->row==row&&b->col==x))return 1;
 return 0;
}
int main(void){CliScreen s,before;OldScreen old;screen_init(&s,25,80);old_screen_init(&old,25,80);uint32_t seed=68000;
 const char *sequences[]={"\033[0m","\033[7m","\033[2J","\033[1;1H","\033[25;80H","\033[?25h","\033[?25l","\033[K","\r\n","hello"};
 for(int i=0;i<200000;i++){
  seed=seed*1664525+1013904223;unsigned char c=seed>>24;const unsigned char *p=&c;size_t n=1;
  if(i%3==0){p=(const unsigned char*)sequences[(seed>>16)%10];n=strlen((const char*)p);}
  before=s;memset(s.dirty,0,sizeof s.dirty);memset(old.dirty,0,sizeof old.dirty);
  screen_feed(&s,p,n);old_screen_feed(&old,p,n);
  for(int row=0;row<s.rows;row++)if(changed(&before,&s,row))assert(s.dirty[row]);
  CliScreen copy=s;memset(copy.dirty,0,sizeof copy.dirty);memset(old.dirty,0,sizeof old.dirty);assert(sizeof copy==sizeof old);assert(!memcmp(&copy,&old,sizeof copy));
 }
 screen_init(&s,25,80);memset(s.dirty,0,sizeof s.dirty);screen_feed(&s,(const unsigned char*)"\033[7m\033[0m",8);for(int i=0;i<s.rows;i++)assert(!s.dirty[i]);
 puts("PASS: 200000 mixed byte/control sequences; unchanged terminal state and every visible change dirties its row; SGR-only does not repaint");
}
'''
(out/'screen-dirty.c').write_text(code)
subprocess.run(['cc','-std=c99','-fsanitize=address,undefined','-I',str(r),str(out/'screen-dirty.c'),'-o',str(out/'screen-dirty')],check=True)
subprocess.run([str(out/'screen-dirty')],check=True)
