#include <assert.h>
#include <stdlib.h>
#include "../../src/commands/terminal/vt100.h"
static Vt100 v;
static char response[1024];static unsigned used;
static int reply(void *ctx,const char *s,unsigned n){(void)ctx;assert(used+n<sizeof response);memcpy(response+used,s,n);used+=n;response[used]=0;return 1;}
static void feed(const char *s){while(*s)assert(vt_byte(&v,(unsigned char)*s++));}
static void reset(void){memset(&v,0,sizeof v);v.reply=reply;vt_reset(&v,6,8);used=0;}
int main(void){
 reset();feed("12345678");assert(v.s.row==0&&v.s.col==7&&v.pending);feed("9");assert(v.s.row==1&&v.s.col==1&&v.s.cells[1][0]=='9');
 feed("\033[?7l\033[1;8HXY");assert(v.s.cells[0][7]=='Y'&&v.s.row==0);
 reset();feed("\033[1;1HA\033[2;1HB\033[3;1HC\033[4;1HD\033[5;1HE\033[6;1HF\033[2;5r\033[5;1H\n");assert(v.s.cells[0][0]=='A'&&v.s.cells[1][0]=='C'&&v.s.cells[3][0]=='E'&&!v.s.cells[4][0]&&v.s.cells[5][0]=='F');
 feed("\033[2;1H\033M");assert(!v.s.cells[1][0]&&v.s.cells[2][0]=='C'&&v.s.cells[5][0]=='F');
 feed("\033[?6h\033[2;3HX");assert(v.s.cells[2][2]=='X');feed("\033[6n");assert(!strcmp(response,"\033[2;4R"));
 feed("\033[7m\0337\033[1;1H\033[0m\0338R");assert(v.s.cells[2][3]=='R'&&v.s.attrs[2][3]);
 reset();feed("abcdef\033[1;3H\033[2P");assert(!memcmp(v.s.cells[0],"abef",4));feed("\033[2@XY");assert(!memcmp(v.s.cells[0],"abXYef",6));
 feed("\033[1;2H\033[4hZ\033[4l");assert(!memcmp(v.s.cells[0],"aZbXYef",7));
 reset();feed("\033)0\016lqqk\017x");assert(!memcmp(v.s.cells[0],"+--+x",5));
 feed("\033[?1h");assert(v.appcursor);feed("\033[?1l");assert(!v.appcursor);
 feed("\033[?25l\033[7mQ\033[27mR");assert(!v.s.cursor&&v.s.attrs[0][5]&&!v.s.attrs[0][6]);
 reset();feed("a\033]52;c;evil\007b\033Pignored\033\\c");assert(!memcmp(v.s.cells[0],"abc",3));
 feed("\033[999999999999;9999999999HX");assert(v.s.row==5&&v.s.col==7);
 feed("\033[1;2;3;4;5;6;7;8;9;10;11;12;13;14;15;16;17H");assert(v.s.row==5&&v.s.col==7);
 reset();feed("a\033%Gb\033( %Gc");assert(!memcmp(v.s.cells[0],"abc",3));
 reset();feed("\033[5n\033[c");assert(!strcmp(response,"\033[0n\033[?1;0c"));
 reset();feed("\033]");for(int i=0;i<4095;i++)assert(vt_byte(&v,'a'));assert(!vt_byte(&v,'a'));
 reset();unsigned seed=1;for(int i=0;i<200000;i++){seed=seed*1664525u+1013904223u;if(!vt_byte(&v,seed>>24))vt_reset(&v,6,8);assert(v.s.row>=0&&v.s.row<6&&v.s.col>=0&&v.s.col<8);assert(v.top>=0&&v.top<=v.bottom&&v.bottom<6);}
 printf("PASS VT: wrap, regions/index/reverse-index, origin/CPR, save/restore, insert/delete, ACS, modes, strings, parameter bounds, 200000 fuzz bytes; model %zu bytes\n",sizeof v);
}
