#include "../display-size.h"
/* Local-only transfer progress. At most one repaint per second; no allocation.
   Caller supplies progress_clock(), progress_tty() and progress_row(text,done). */
static unsigned long progress_started,progress_last;
static int progress_enabled,progress_shown;
static void progress_begin(void){progress_started=progress_last=progress_clock();progress_enabled=progress_tty();progress_shown=0;}
static unsigned progress_percent(unsigned long n,unsigned long total){
 if(!total||n>=total)return 100;unsigned lo=0,hi=100;
 while(lo+1<hi){unsigned m=(lo+hi)/2;unsigned long threshold=(total/100)*m+((total%100)*m+99)/100;if(n>=threshold)lo=m;else hi=m;}return lo;
}
static void progress_update(unsigned long bytes,unsigned long total,int known,int done){
 unsigned long now=progress_clock(),seconds=(now-progress_started)/60;
 if(!progress_enabled||(!done&&now-progress_last<60))return;
 progress_last=now;char bar[13],line[128];unsigned pc=known?progress_percent(bytes,total):0;
 for(unsigned i=0;i<12;i++)bar[i]=known?(i*100<pc*12?'=':' '):(i==seconds%12?'>':' ');bar[12]=0;
 unsigned long rate=seconds?bytes/seconds:0;char amount[32],speed[32];display_size(amount,bytes);display_size(speed,rate);
 if(known)snprintf(line,sizeof line,"%3u%% [%s] %s %s/s %lus",pc,bar,amount,speed,seconds);
 else snprintf(line,sizeof line,"[ %s ] %s %s/s %lus",bar,amount,speed,seconds);
 progress_row(line,done);progress_shown=1;
}
