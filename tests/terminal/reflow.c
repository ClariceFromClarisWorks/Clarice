#include "reflow.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static TerminalReflow a,b,c;
int main(void){
 strcpy(a.lines[0],"abcdefghij");a.wraps[0]=1;strcpy(a.lines[1],"klmnop");strcpy(a.lines[2],"hard break");
 terminal_reflow(&b,a.lines,a.wraps,0,2,0,5,128);
 assert(b.end==5&&b.column==5);
 assert(!strcmp(b.lines[0],"abcde")&&!strcmp(b.lines[1],"fghij")&&!strcmp(b.lines[2],"klmno")&&!strcmp(b.lines[3],"p"));
 assert(b.wraps[0]&&b.wraps[1]&&b.wraps[2]&&!b.wraps[3]&&b.wraps[4]&&!b.wraps[5]);
 terminal_reflow(&c,b.lines,b.wraps,b.oldest,b.end,0,20,128);
 assert(c.end==1&&!strcmp(c.lines[0],"abcdefghijklmnop")&&!strcmp(c.lines[1],"hard break"));
 memset(&a,0,sizeof a);strcpy(a.lines[0],"12345");strcpy(a.lines[2],"x  ");
 terminal_reflow(&b,a.lines,a.wraps,0,2,1,5,128);
 assert(b.end==2&&b.column==3&&!strcmp(b.lines[0],"12345")&&!b.lines[1][0]&&!strcmp(b.lines[2],"x  ")&&b.anchor==1);
 memset(&a,0,sizeof a);for(int i=0;i<128;i++){memset(a.lines[i],'A'+i%26,80);a.wraps[i]=0;}
 terminal_reflow(&b,a.lines,a.wraps,0,127,100,40,32);
 assert(b.end==255&&b.oldest==224&&b.anchor==224&&b.column==40);
 for(long i=b.oldest;i<=b.end;i++){assert(strlen(b.lines[i%128])==40);assert(b.wraps[i%128]==(i%2==0));}
 puts("Reflow PASS: soft joins, hard breaks, exact-width lines, blanks, spaces, bounded eviction and anchored views.");
}
