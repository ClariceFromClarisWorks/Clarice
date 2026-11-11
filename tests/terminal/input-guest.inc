/* _PostEvent register interface; Multiversal lacks the POSTEVENT glue. */
static int probe_post(unsigned char c){register long kind __asm__("a0")=keyDown;register long message __asm__("d0")=c;__asm__ volatile(".word 0xa02f" : "+a"(kind), "+d"(message) : : "d1","a1","cc","memory");return (short)message;}
/* Test-build-only producer through the guest Event Manager. No runtime service. */
static void probe_write(const char *s){char path[PATH_CAP];unsigned char p[256];short f;shell_temp_path(path,"qol2-input.txt");pstr(p,path);if(HOpen(0,0,p,fsWrPerm,&f)){HCreate(0,0,p,'CLRC','TEXT');if(HOpen(0,0,p,fsWrPerm,&f))return;}SetFPos(f,fsFromLEOF,0);long n=strlen(s);FSWrite(f,&n,s);FSClose(f);FlushVol(0,0);}
#ifdef CLARICE_INPUT_SAFETY_TEST
#include "input-safety-guest.inc"
#endif
static void input_probe_tick(void){
 static int stage=-1,posted,attempted;static unsigned long start,paint0,end;static long heap;
 static const int periods[]={7,5,1,0};char report[256];
 if(stage==4)return;
 if(stage<0){probe_write("QOL2 INPUT PROBE\n");stage=0;start=TickCount();heap=FreeMem();input[0]=0;cursor=0;paint0=paint_ticks;probe_events=probe_age=probe_age_max=0;}
 if(attempted<240){unsigned long elapsed=TickCount()-start;int due=periods[stage]?(int)(elapsed/periods[stage])+1:attempted+16;if(due>240)due=240;
  while(attempted<due){int e=probe_post('a'+attempted%26);if(!e)posted++;attempted++;}if(attempted==240)end=TickCount();return;}
 if(TickCount()-end<120)return;
 int good=strlen(input)==240;for(int i=0;i<(int)strlen(input);i++)if(input[i]!='a'+i%26)good=0;
 snprintf(report,sizeof report,"stage=%d chars=240 posted=%d received=%lu retained=%lu correct=%d elapsed=%lu paint=%lu age_sum=%lu age_max=%lu heap_delta=%ld\n",stage,posted,probe_events,(unsigned long)strlen(input),good,TickCount()-start-120,paint_ticks-paint0,probe_age,probe_age_max,FreeMem()-heap);probe_write(report);
 stage++;posted=attempted=0;input[0]=0;cursor=0;dirty=1;start=TickCount();paint0=paint_ticks;probe_events=probe_age=probe_age_max=0;
 if(stage==4){
#ifdef CLARICE_INPUT_SAFETY_TEST
 input_safety_probe();
#endif
 probe_write("PROBE-DONE\n");term_puts("INPUT-PROBE-DONE\n");}
}
