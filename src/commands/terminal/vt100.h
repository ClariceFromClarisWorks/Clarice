/* Invocation-owned VT100/ANSI character model. Independent implementation from
 * DEC's VT100 programming manual; reuses Clarice's bounded screen storage.
 * Rendering stays in app_screen_open/cli_write, never QuickDraw here. */
#ifndef CLARICE_VT100_H
#define CLARICE_VT100_H
#include "../../ports/terminal/screen.h"
#include <stdio.h>
typedef struct {
 CliScreen s;
 int top,bottom,wrap,pending,origin,insert,appcursor,keypad,newline;
 int saved_row,saved_col,saved_reverse,saved_origin,saved_wrap,saved_charset,saved_g0,saved_g1;
 int g0,g1,charset,global_reverse,parser,intermediate,discard,argc,args[16],priv;
 unsigned char tabs[SCREEN_COLS];
 unsigned controls;
 int (*reply)(void *,const char *,unsigned);void *context;
} Vt100;
static int vt_bound(int n,int low,int high){return n<low?low:n>high?high:n;}
static void vt_home(Vt100 *v){v->s.row=v->origin?v->top:0;v->s.col=v->pending=0;}
static void vt_reset(Vt100 *v,int rows,int cols){
 int (*reply)(void*,const char*,unsigned)=v->reply;void *context=v->context;
 memset(v,0,sizeof *v);v->reply=reply;v->context=context;screen_init(&v->s,rows,cols);
 v->bottom=v->s.rows-1;v->wrap=v->saved_wrap=1;for(int i=8;i<v->s.cols;i+=8)v->tabs[i]=1;
}
static void vt_blank(Vt100 *v,int row,int first,int end){screen_erase(&v->s,row,first,end);}
static void vt_scroll(Vt100 *v,int top,int bottom,int count){
 CliScreen *s=&v->s;int n=vt_bound(count<0?-count:count,1,bottom-top+1);
 int retained=bottom-top+1-n;
 if(count>0){memmove(s->cells[top],s->cells[top+n],retained*SCREEN_COLS);memmove(s->attrs[top],s->attrs[top+n],retained*SCREEN_COLS);for(int r=bottom-n+1;r<=bottom;r++)vt_blank(v,r,0,s->cols);}
 else{memmove(s->cells[top+n],s->cells[top],retained*SCREEN_COLS);memmove(s->attrs[top+n],s->attrs[top],retained*SCREEN_COLS);for(int r=top;r<top+n;r++)vt_blank(v,r,0,s->cols);}
 for(int r=top;r<=bottom;r++)s->dirty[r]=1;
}
static void vt_index(Vt100 *v,int direction){
 if(direction>0){if(v->s.row==v->bottom)vt_scroll(v,v->top,v->bottom,1);else if(v->s.row<v->s.rows-1)v->s.row++;}
 else{if(v->s.row==v->top)vt_scroll(v,v->top,v->bottom,-1);else if(v->s.row)v->s.row--;}
 v->pending=0;
}
static void vt_save(Vt100 *v){v->saved_row=v->s.row;v->saved_col=v->s.col;v->saved_reverse=v->s.reverse;v->saved_origin=v->origin;v->saved_wrap=v->wrap;v->saved_charset=v->charset;v->saved_g0=v->g0;v->saved_g1=v->g1;}
static void vt_restore(Vt100 *v){v->s.row=v->saved_row;v->s.col=v->saved_col;v->s.reverse=v->saved_reverse;v->origin=v->saved_origin;v->wrap=v->saved_wrap;v->charset=v->saved_charset;v->g0=v->saved_g0;v->g1=v->saved_g1;v->pending=0;}
static void vt_chars(Vt100 *v,int n,int insert){
 CliScreen *s=&v->s;n=vt_bound(n,1,s->cols-s->col);int left=s->cols-s->col-n;
 if(insert){memmove(s->cells[s->row]+s->col+n,s->cells[s->row]+s->col,left);memmove(s->attrs[s->row]+s->col+n,s->attrs[s->row]+s->col,left);vt_blank(v,s->row,s->col,s->col+n);}
 else{memmove(s->cells[s->row]+s->col,s->cells[s->row]+s->col+n,left);memmove(s->attrs[s->row]+s->col,s->attrs[s->row]+s->col+n,left);vt_blank(v,s->row,s->cols-n,s->cols);}
}
static unsigned char vt_graphic(unsigned char c){
 /* ASCII fallbacks for DEC special graphics on installed Macintosh fonts. */
 if(strchr("jklmntuvw",c))return '+';if(c=='q'||c=='o'||c=='p'||c=='r'||c=='s')return '-';if(c=='x')return '|';if(c=='a')return '#';if(c=='`')return '*';if(c>=95&&c<=126)return '.';return c;
}
static int vt_response(Vt100 *v,const char *s){return !v->reply||v->reply(v->context,s,(unsigned)strlen(s));}
static int vt_csi(Vt100 *v,unsigned char c){
 CliScreen *s=&v->s;int a=v->args[0],b=v->args[1],n=a?a:1,low=v->origin?v->top:0,high=v->origin?v->bottom:s->rows-1;
 if(v->discard||v->intermediate)return 1;
 if(v->priv&&c!='h'&&c!='l')return 1;
 switch(c){
 case 'H':case 'f':s->row=vt_bound((a?a:1)-1+low,low,high);s->col=vt_bound((b?b:1)-1,0,s->cols-1);v->pending=0;break;
 case 'A':s->row=vt_bound(s->row-n,low,high);v->pending=0;break;
 case 'B':case 'e':s->row=vt_bound(s->row+n,low,high);v->pending=0;break;
 case 'C':case 'a':s->col=vt_bound(s->col+n,0,s->cols-1);v->pending=0;break;
 case 'D':s->col=vt_bound(s->col-n,0,s->cols-1);v->pending=0;break;
 case 'E':case 'F':s->row=vt_bound(s->row+(c=='E'?n:-n),low,high);s->col=v->pending=0;break;
 case 'G':case '`':s->col=vt_bound(n-1,0,s->cols-1);v->pending=0;break;
 case 'd':s->row=vt_bound(n-1+low,low,high);v->pending=0;break;
 case 'J':
  if(a<=2)for(int r=0;r<s->rows;r++)if(a==2||(!a&&r>=s->row)||(a==1&&r<=s->row))vt_blank(v,r,r==s->row&&!a?s->col:0,r==s->row&&a==1?s->col+1:s->cols);
  v->pending=0;break;
 case 'K':if(a<=2)vt_blank(v,s->row,a==0?s->col:0,a==1?s->col+1:s->cols);v->pending=0;break;
 case 'X':vt_blank(v,s->row,s->col,vt_bound(s->col+n,0,s->cols));v->pending=0;break;
 case 'P':case '@':vt_chars(v,n,c=='@');v->pending=0;break;
 case 'L':case 'M':if(s->row>=v->top&&s->row<=v->bottom)vt_scroll(v,s->row,v->bottom,c=='M'?n:-n);v->pending=0;break;
 case 'S':case 'T':vt_scroll(v,v->top,v->bottom,c=='S'?n:-n);v->pending=0;break;
 case 'r':if(!a)a=1;if(!b)b=s->rows;if(a<b&&b<=s->rows){v->top=a-1;v->bottom=b-1;vt_home(v);}break;
 case 'm':for(int i=0;i<=v->argc;i++){int x=v->args[i];if(x==0||x==27)s->reverse=0;else if(x==7)s->reverse=1;}break;
 case 'g':if(a==0)v->tabs[s->col]=0;else if(a==3)memset(v->tabs,0,sizeof v->tabs);break;
 case 'h':case 'l':for(int i=0;i<=v->argc;i++){int x=v->args[i],on=c=='h';if(v->priv){
   if(x==1)v->appcursor=on;else if(x==6){v->origin=on;vt_home(v);}else if(x==7){v->wrap=on;v->pending=0;}
   else if(x==25)s->cursor=on;else if(x==5&&v->global_reverse!=on){v->global_reverse=on;memset(s->dirty,1,sizeof s->dirty);}
  }else if(x==4)v->insert=on;else if(x==20)v->newline=on;}break;
 case 's':vt_save(v);break;case 'u':vt_restore(v);break;
 case 'c':if(!a)return vt_response(v,"\033[?1;0c");break;
 case 'n':if(a==5)return vt_response(v,"\033[0n");if(a==6){char out[32];snprintf(out,sizeof out,"\033[%d;%dR",s->row-low+1,s->col+1);return vt_response(v,out);}break;
 }
 return 1;
}
static int vt_byte(Vt100 *v,unsigned char c){
 CliScreen *s=&v->s;
 /* String payloads (including OSC clipboard/title) are consumed, never acted on
    or rendered. Bound controls so hostile unterminated streams fail clearly. */
 if(v->parser&&++v->controls>4096)return 0;
 if(v->parser==4||v->parser==5){if(c==7||c==24||c==26||(v->parser==5&&c=='\\'))v->parser=0;else v->parser=c==27?5:4;return 1;}
 if(c==24||c==26){v->parser=0;return 1;}
 if(c==27){v->parser=1;v->controls=0;return 1;}
 if(c<32){
  if(c==8){if(s->col)s->col--;v->pending=0;}
  else if(c==9){int col=s->col+1;while(col<s->cols-1&&!v->tabs[col])col++;s->col=vt_bound(col,0,s->cols-1);v->pending=0;}
  else if(c==10||c==11||c==12){vt_index(v,1);if(v->newline)s->col=0;}
  else if(c==13)s->col=v->pending=0;else if(c==14)v->charset=1;else if(c==15)v->charset=0;
  return 1;
 }
 if(c==127)return 1;
 if(v->parser==1){v->parser=0;
  if(c=='['){v->parser=2;v->argc=v->priv=v->discard=v->intermediate=0;memset(v->args,0,sizeof v->args);}
  else if(c==']'||c=='P'||c=='^'||c=='_')v->parser=4;
  else if(c>=32&&c<=47){v->parser=3;v->intermediate=c;}
  else if(c=='7')vt_save(v);else if(c=='8')vt_restore(v);
  else if(c=='D')vt_index(v,1);else if(c=='M')vt_index(v,-1);else if(c=='E'){vt_index(v,1);s->col=0;}
  else if(c=='H')v->tabs[s->col]=1;else if(c=='=')v->keypad=1;else if(c=='>')v->keypad=0;
  else if(c=='c')vt_reset(v,s->rows,s->cols);else if(c=='Z')return vt_response(v,"\033[?1;0c");
  return 1;
 }
 if(v->parser==3){
  /* Consume unsupported ESC intermediates through their final byte. */
  if(c>=32&&c<=47){v->intermediate=0;return 1;}
  v->parser=0;
  if(v->intermediate=='(')v->g0=c=='0';else if(v->intermediate==')')v->g1=c=='0';
  else if(v->intermediate=='#'&&c=='8'){for(int r=0;r<s->rows;r++){memset(s->cells[r],'E',s->cols);memset(s->attrs[r],0,s->cols);s->dirty[r]=1;}}return 1;
 }
 if(v->parser==2){
  if(c=='?'&&!v->argc&&!v->args[0]){v->priv=1;return 1;}
  if(c>='0'&&c<='9'){int *a=&v->args[v->argc];*a=vt_bound(*a*10+c-'0',0,9999);return 1;}
  if(c==';'){if(v->argc<15)v->argc++;else v->discard=1;return 1;}
  if(c>=32&&c<=47){v->intermediate=c;return 1;}
  if(c>=64&&c<=126){v->parser=0;return vt_csi(v,c);}v->discard=1;return 1;
 }
 if(v->pending&&v->wrap){s->col=0;vt_index(v,1);}v->pending=0;
 if(v->insert)vt_chars(v,1,1);
 if(v->charset?v->g1:v->g0)c=vt_graphic(c);
 /* VT100 is a 7-bit contract, not UTF-8. Never treat high bytes as controls. */
 if(c>=128)c='?';
 if(s->cells[s->row][s->col]!=c||s->attrs[s->row][s->col]!=s->reverse)s->dirty[s->row]=1;
 s->cells[s->row][s->col]=c;s->attrs[s->row][s->col]=s->reverse;
 if(s->col==s->cols-1)v->pending=v->wrap;else s->col++;
 return 1;
}
#endif
