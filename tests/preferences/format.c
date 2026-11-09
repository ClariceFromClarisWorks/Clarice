#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../src/preferences-format.h"
int main(void){
 struct ClaricePrefs p={"Monaco",9,0,1,128,0,340,280,4,40,0},q=p;
 char text[PREFS_TEXT_MAX+2];int n=prefs_format(text,sizeof text,&p);assert(n>0&&prefs_parse(&q,text,n));assert(!memcmp(&p,&q,sizeof p));
 strcpy(text," # comment\r\n font = New York \rsize=12\ncolours=dark\npaging=100\nscrollback=32\nrenderer=c\nwidth=300\nheight=180\nx=12\ny=45\nfill=1\n");assert(prefs_parse(&q,text,strlen(text)));assert(!strcmp(q.font,"New York")&&q.size==12&&q.inverse==1&&q.paging==2&&q.retained==32&&q.renderer==1&&q.width==300&&q.height==180&&q.x==12&&q.y==45&&q.fill==1);
 p=q;strcpy(text,"size=999999999999999999999\nscrollback=100\nrenderer=fast\ncolours=blue\npaging=-1\nx=-2\nfill=2\nwidth=2\nheight=0\nfont=\nunknown=$(rm stuff)\n");assert(prefs_parse(&q,text,strlen(text)));assert(!memcmp(&p,&q,sizeof p));
 memset(text,'x',sizeof text);assert(!prefs_parse(&q,text,PREFS_TEXT_MAX+1));assert(!memcmp(&p,&q,sizeof p));
 strcpy(text,"size=14");text[8]=0;assert(!prefs_parse(&q,text,9));assert(!memcmp(&p,&q,sizeof p));
 memset(text,'x',PREFS_TEXT_MAX);assert(prefs_parse(&q,text,PREFS_TEXT_MAX));
 strcpy(text,"size=6\nsize=24\nrenderer=asm\npaging=off\ncolours=light\nfill=0");assert(prefs_parse(&q,text,strlen(text)));assert(q.size==24&&q.renderer==2&&q.paging==0&&q.inverse==0&&!q.fill);
 puts("PASS: roundtrip, CR/LF, whitespace, all fields, invalid values, overflow, binary, size cap, duplicate keys");
}
