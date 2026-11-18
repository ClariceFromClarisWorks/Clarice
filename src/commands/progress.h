#include <stdint.h>
/* Invocation-local progress, no allocation. Callers supply clock() in 60 Hz
 * ticks, tty(), columns() and row(text,finish). Positive done means success,
 * negative means failure, zero ongoing. No protocol/output-stream bytes here. */
static unsigned long progress_started,progress_last;
static int progress_enabled,progress_shown;
static void progress_begin(void){progress_started=progress_clock();progress_last=progress_started-60;progress_enabled=progress_tty();progress_shown=0;}
static unsigned progress_percent(unsigned long n,unsigned long total){
 if(!total||n>=total)return 100;unsigned lo=0,hi=100;
 while(lo+1<hi){unsigned m=(lo+hi)/2;unsigned long threshold=(total/100)*m+((total%100)*m+99)/100;if(n>=threshold)lo=m;else hi=m;}return lo;
}
/* 32-bit counters suffice for native HFS transfers. Exact integer formatting
 * avoids floating point and 64-bit division on the 68000. */
static void progress_size(char out[24],unsigned long n){
 unsigned shift=n>=1073741824UL?30:n>=1048576UL?20:n>=1024?10:0;
 if(!shift){snprintf(out,24,"%lu B",n);return;}
 uint32_t base=1UL<<shift,rem=(uint32_t)n&(base-1),q=base>>6;
 /* Split before multiplying: even for GiB the largest intermediate is
    <195*(1<<24), within uint32_t. Explicit width keeps host tests honest. */
 unsigned high=(rem/q)*100;
 unsigned fraction=high/64+(((high%64)*q+(rem%q)*100+base/2)>>shift);
 unsigned whole=n>>shift;
 if(fraction==100){whole++;fraction=0;}
 if(whole==1024&&shift<30){whole=1;shift+=10;}
 snprintf(out,24,"%u.%02u %s",whole,fraction,shift==10?"KiB":shift==20?"MiB":"GiB");
}
static void progress_paint(const char *info,unsigned pc,int known,int done,unsigned long seconds,int width){
 char line[132];
 int spare=width-(int)strlen(info),at=0;
 if(spare>=6){int bar=spare-3;line[at++]='[';unsigned filled=known?(pc*(unsigned)bar)/100:0;
  for(int i=0;i<bar;i++)line[at++]=known?(i<(int)filled?'=':' '):(i==(int)(seconds%bar)?'>':' ');
  line[at++]=']';line[at++]=' ';}
 unsigned n=strlen(info);if(n>(unsigned)(width-at))n=width-at;memcpy(line+at,info,n);line[at+n]=0;
 progress_row(line,done!=0);progress_shown=1;
}
static void progress_update(unsigned long bytes,unsigned long total,int known,int done){
 unsigned long now=progress_clock(),seconds=(now-progress_started)/60;
 if(!progress_enabled||(!done&&now-progress_last<60))return;
 progress_last=now;int width=progress_columns()-1;if(width<1)width=1;if(width>130)width=130;
 char amount[24],size[24],info[96];unsigned pc=known?progress_percent(bytes,total):0;

#ifdef PROGRESS_PENDING_COMMIT
 if(done<=0&&pc==100)pc=99;
#endif
 progress_size(amount,bytes);progress_size(size,total);
 if(known)snprintf(info,sizeof info,"%3u%% %s / %s",pc,amount,size);
 else snprintf(info,sizeof info,"%s / ?",amount);
 /* Retain amounts and percentage before a bar or speed. Narrow windows omit
    optional fields, never wrap a progress row into the scrollback. */
 if((int)strlen(info)>width){if(known)snprintf(info,sizeof info,"%u%% %s",pc,amount);else snprintf(info,sizeof info,"%s",amount);}
 if((int)strlen(info)>width){if(known)snprintf(info,sizeof info,"%u%%",pc);else strcpy(info,"...");}
#ifndef PROGRESS_NO_RATE
 if(seconds){char speed[24];progress_size(speed,bytes/seconds);if(strlen(info)+strlen(speed)+4<=(unsigned)width){strcat(info," ");strcat(info,speed);strcat(info,"/s");}}
#endif
 if(done<0&&strlen(info)+2<=(unsigned)width)strcat(info," !");
 progress_paint(info,pc,known,done,seconds,width);
}
/* Unknown totals: use the same moving bar and width bounds for item counts. */
static inline void progress_message(const char *info,int done){
 unsigned long now=progress_clock();
 if(!progress_enabled||(!done&&now-progress_last<60))return;
 progress_last=now;int width=progress_columns()-1;if(width<1)width=1;if(width>130)width=130;
 progress_paint(info,done>0?100:0,done>0,done,(now-progress_started)/60,width);
}
