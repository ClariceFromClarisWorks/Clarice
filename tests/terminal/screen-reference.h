/* Bounded virtual terminal for Clarice's advertised terminfo capabilities.
   No filesystem or QuickDraw dependencies: the parser is host-testable. */
#ifndef CLARICE_SCREEN_H
#define CLARICE_SCREEN_H
#include <string.h>
#define SCREEN_ROWS 40
#define SCREEN_COLS 132
typedef struct {unsigned char cells[SCREEN_ROWS][SCREEN_COLS],attrs[SCREEN_ROWS][SCREEN_COLS],dirty[SCREEN_ROWS];int rows,cols,row,col,reverse,cursor,state,private_mode,params[8],param;unsigned char keys[128];unsigned head,count;} CliScreen;
static void screen_init(CliScreen *s,int rows,int cols){memset(s,0,sizeof *s);s->rows=rows<1?1:rows>SCREEN_ROWS?SCREEN_ROWS:rows;s->cols=cols<1?1:cols>SCREEN_COLS?SCREEN_COLS:cols;s->cursor=1;memset(s->dirty,1,sizeof s->dirty);}
static void screen_erase(CliScreen *s,int row,int from,int to){memset(s->cells[row]+from,0,to-from);memset(s->attrs[row]+from,0,to-from);s->dirty[row]=1;}
static void screen_scroll(CliScreen *s){memmove(s->cells,s->cells+1,(s->rows-1)*SCREEN_COLS);memmove(s->attrs,s->attrs+1,(s->rows-1)*SCREEN_COLS);screen_erase(s,s->rows-1,0,s->cols);memset(s->dirty,1,sizeof s->dirty);}
static void screen_feed(CliScreen *s,const unsigned char *p,unsigned long n){
 while(n--){unsigned char c=*p++;int old=s->row;s->dirty[old]=1;
 if(s->state==1){s->state=c=='['?2:0;s->private_mode=0;s->param=0;memset(s->params,0,sizeof s->params);continue;}
 if(s->state==2){if(c=='?'){s->private_mode=1;continue;}if(c>='0'&&c<='9'){int *v=&s->params[s->param];if(*v<1000)*v=*v*10+c-'0';continue;}if(c==';'){if(s->param<7)s->param++;continue;}
  int a=s->params[0],b=s->params[1];if(c=='H'||c=='f'){s->row=a?a-1:0;s->col=b?b-1:0;}
  else if(c=='A')s->row-=a?a:1;else if(c=='B')s->row+=a?a:1;else if(c=='C')s->col+=a?a:1;else if(c=='D')s->col-=a?a:1;
  else if(c=='J'){for(int r=0;r<s->rows;r++)if(a==2||(!a&&r>=s->row)||(a==1&&r<=s->row))screen_erase(s,r,r==s->row&&!a?s->col:0,r==s->row&&a==1?s->col+1:s->cols);}
  else if(c=='K')screen_erase(s,s->row,a==0?s->col:0,a==1?s->col+1:s->cols);
  else if(c=='m'){for(int i=0;i<=s->param;i++)if(s->params[i]==0||s->params[i]==27)s->reverse=0;else if(s->params[i]==7)s->reverse=1;}
  else if(s->private_mode&&a==25&&(c=='h'||c=='l'))s->cursor=c=='h';
  s->state=0;
 }else if(c==27)s->state=1;
 else if(c=='\r')s->col=0;
 else if(c=='\n'){if(++s->row>=s->rows){screen_scroll(s);s->row=s->rows-1;}}
 else if(c=='\b'){if(s->col)s->col--;}
 else if(c=='\t'){s->col=(s->col+8)&~7;}
 else if(c>=32&&c!=127){s->cells[s->row][s->col]=c;s->attrs[s->row][s->col]=s->reverse;if(s->col<s->cols-1)s->col++;}
 if(s->row<0)s->row=0;if(s->row>=s->rows)s->row=s->rows-1;if(s->col<0)s->col=0;if(s->col>=s->cols)s->col=s->cols-1;s->dirty[s->row]=1;
 }
}
#endif
