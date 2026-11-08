#include "reflow.h"
#include <string.h>
static void advance(TerminalReflow *out,int soft){out->wraps[out->end%TERM_ROWS]=(unsigned char)soft;out->end++;out->column=0;memset(out->lines[out->end%TERM_ROWS],0,TERM_COLS);out->wraps[out->end%TERM_ROWS]=0;}
void terminal_reflow(TerminalReflow *out,const char lines[TERM_ROWS][TERM_COLS],const unsigned char *wraps,long first,long last,long anchor,int width,int retained){
 memset(out,0,sizeof *out);
 for(long row=first;row<=last;row++){
  const char *s=lines[row%TERM_ROWS];int n=0;while(n<TERM_COLS&&s[n])n++;
  if(row==anchor)out->anchor=out->end+(out->column==width&&n>0);
  for(int i=0;i<n;i++){if(out->column==width)advance(out,1);out->lines[out->end%TERM_ROWS][out->column++]=s[i];}
  if(row<last&&!wraps[row%TERM_ROWS])advance(out,0);
 }
 out->oldest=out->end-retained+1;if(out->oldest<0)out->oldest=0;
 if(out->anchor<out->oldest)out->anchor=out->oldest;
}
