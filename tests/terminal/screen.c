#include <assert.h>
#include <stdio.h>
#include "../../src/ports/terminal/screen.h"
static void feed(CliScreen *s,const char *p){screen_feed(s,(const unsigned char*)p,strlen(p));}
int main(void){CliScreen s;screen_init(&s,4,8);feed(&s,"abc\033[2;3HX\033[7mR\033[0mN");assert(s.cells[0][0]=='a'&&s.cells[1][2]=='X'&&s.attrs[1][3]==1&&s.attrs[1][4]==0);feed(&s,"\033[H\033[2J");for(int r=0;r<4;r++)for(int c=0;c<8;c++)assert(!s.cells[r][c]&&!s.attrs[r][c]);feed(&s,"\033[9999;9999H!");assert(s.row==3&&s.col==7&&s.cells[3][7]=='!');feed(&s,"\n");assert(s.cells[2][7]=='!');feed(&s,"\033[?25l");assert(!s.cursor);feed(&s,"\033[?25h");assert(s.cursor);
 unsigned long seed=1;for(int i=0;i<100000;i++){seed=seed*1664525+1013904223;unsigned char c=seed>>24;screen_feed(&s,&c,1);assert(s.row>=0&&s.row<4&&s.col>=0&&s.col<8);}
 puts("PASS: cursor, erasure, inverse attributes, scrolling, visibility and 100000 bounded parser bytes");}
