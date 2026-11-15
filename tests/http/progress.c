#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
static unsigned long clock_value;static int tty,calls,finished,width=80;static char row[132];
static unsigned long progress_clock(void){return clock_value;}
static int progress_tty(void){return tty;}
static int progress_columns(void){return width;}
static void progress_row(const char *s,int done){assert(strlen(s)<sizeof row);assert(strlen(s)<(unsigned)width||width<2);strcpy(row,s);calls++;finished+=done;}
#include "../../src/commands/progress.h"
int main(void){
 tty=1;progress_begin();progress_update(0,1000,1,0);assert(calls==1&&strstr(row,"0%"));
 for(clock_value=0;clock_value<60;clock_value++)progress_update(100,1000,1,0);assert(calls==1);
 progress_update(500,1000,1,0);assert(calls==2&&strstr(row,"50%")&&strstr(row,"500 B / 1000 B"));
 progress_update(1000,1000,1,1);assert(calls==3&&finished==1&&strstr(row,"100%"));
 calls=finished=0;tty=0;progress_begin();clock_value+=120;progress_update(100,100,1,1);assert(!calls);
 tty=1;progress_begin();clock_value+=60;progress_update(512,0,0,0);assert(calls==1&&!strchr(row,'%')&&strstr(row,"512 B / ?"));
 char size[24];progress_size(size,1024);assert(!strcmp(size,"1.00 KiB"));progress_size(size,1048576);assert(!strcmp(size,"1.00 MiB"));progress_size(size,0xffffffffUL);assert(!strcmp(size,"4.00 GiB"));
 for(unsigned long value=1024;value<=0xffffffffUL;){
  progress_size(size,value);
  unsigned shift=value>=1073741824UL?30:value>=1048576UL?20:10;
  unsigned long long base=1ULL<<shift,scaled=((unsigned long long)value*100+base/2)/base;
  unsigned whole=scaled/100,frac=scaled%100;if(whole==1024&&shift<30){whole=1;shift+=10;}
  char expected[24];snprintf(expected,sizeof expected,"%u.%02u %s",whole,frac,shift==10?"KiB":shift==20?"MiB":"GiB");assert(!strcmp(size,expected));
  if(value>0xffffffffUL-104729)break;value+=104729;
 }
 progress_size(size,0x7fffffffUL);assert(!strcmp(size,"2.00 GiB"));
 progress_size(size,0xffffffffUL);assert(!strcmp(size,"4.00 GiB"));
 progress_update(256000,512000,1,1);assert(strstr(row,"250.00 KiB / 500.00 KiB"));
 progress_update(500,1000,1,-1);assert(strstr(row,"50%")&&strchr(row,'!'));
 for(width=1;width<=200;width++)for(int known=0;known<2;known++)for(int done=-1;done<=1;done++){clock_value+=60;progress_update(1073741823UL,2147483647UL,known,done);}
 assert(progress_percent(0,2147483647UL)==0);assert(progress_percent(1073741823UL,2147483647UL)==49);assert(progress_percent(1073741824UL,2147483647UL)==50);assert(progress_percent(2147483647UL,2147483647UL)==100);
 width=80;progress_update(0,0,1,1);assert(strstr(row,"100%")&&strstr(row,"0 B / 0 B"));
 puts("PASS: width 1..200, current/total IEC sizes, unknown/zero totals, overflow-safe percentages, failure/final rows, throttle, redirected suppression");return 0;
}
