#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
static unsigned long clock_value;static int tty,calls,finished;static char row[128];
static unsigned long progress_clock(void){return clock_value;}
static int progress_tty(void){return tty;}
static void progress_row(const char *s,int done){assert(strlen(s)<sizeof row);strcpy(row,s);calls++;finished+=done;}
#include "../../src/commands/progress.h"
int main(void){
 tty=1;progress_begin();for(clock_value=0;clock_value<60;clock_value++)progress_update(100,1000,1,0);assert(calls==0);
 progress_update(500,1000,1,0);assert(calls==1&&strstr(row,"50%"));
 progress_update(1000,1000,1,1);assert(calls==2&&finished==1&&strstr(row,"100%"));
 calls=finished=0;tty=0;progress_begin();clock_value+=120;progress_update(100,100,1,1);assert(!calls);
 tty=1;progress_begin();clock_value+=60;progress_update(512,0,0,0);assert(calls==1&&!strchr(row,'%')&&strstr(row,"512 B"));
 assert(progress_percent(0,2147483647UL)==0);assert(progress_percent(1073741823UL,2147483647UL)==49);assert(progress_percent(1073741824UL,2147483647UL)==50);assert(progress_percent(2147483647UL,2147483647UL)==100);
 puts("PASS: once-per-second updates, final row, redirected stderr suppressed, unknown total, bounded percentage arithmetic");return 0;
}
