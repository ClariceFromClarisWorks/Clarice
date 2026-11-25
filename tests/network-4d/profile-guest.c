/* Disposable profiling wrapper around the actual client and native transport.
 * Counts only foreground calls. TickCount is elapsed guest time, not a CPU timer;
 * per-yield instrumentation adds overhead, so normal-client timings are separate. */
#include <Multiverse.h>
#include <Memory.h>
#include <stdio.h>
extern int app_yield(void),cli_pause_ticks(unsigned long);
extern void say(const char *);
extern int cli_file_open(const char *,const char *),cli_file_close(int);
extern long cli_file_write(int,const void *,unsigned long);
static unsigned long checks,sleeps,status_calls,empty_calls,recv_calls,send_calls;
static unsigned long last_check,max_gap,yield_ticks,pause_ticks;
static long system_before,system_low;
static long system_free(void){THz old=GetZone();SetZone(LMGetSysZone());long n=FreeMem();SetZone(old);return n;}
static int measured_yield(void){unsigned long t=TickCount();if(last_check&&t-last_check>max_gap)max_gap=t-last_check;last_check=t;checks++;int e=app_yield();yield_ticks+=TickCount()-t;return e;}
static int measured_pause(unsigned long n){sleeps++;unsigned long t=TickCount();int e=cli_pause_ticks(n);pause_ticks+=TickCount()-t;return e;}
static OSErr measured_control(ParmBlkPtr p){TCPiopb *b=(TCPiopb*)p;OSErr e=PBControlSync(p);if(b->csCode==TCPStatus){status_calls++;if(!e&&!b->csParam.status.amtUnreadData)empty_calls++;}if(b->csCode==TCPCreate){long n=system_free();if(n<system_low)system_low=n;}return e;}
static OSErr measured_async(ParmBlkPtr p){TCPiopb *b=(TCPiopb*)p;if(b->csCode==TCPRcv)recv_calls++;if(b->csCode==TCPSend)send_calls++;return PBControlAsync(p);}
#define app_yield measured_yield
#define cli_pause_ticks measured_pause
#define PBControlSync measured_control
#define PBControlAsync measured_async
#include "../../src/commands/vendor/mactcp/nettcp.c"
#define clarice_main client_main
#if defined(PROFILE_WGET)
#include "../../src/commands/wget.c"
#elif defined(PROFILE_FTP)
#include "../../src/commands/ftp.c"
#else
#include "../../src/commands/telnet.c"
#endif
#undef clarice_main
int clarice_main(int argc,char **argv){system_before=system_low=system_free();unsigned long start=TickCount();int result=client_main(argc,argv);unsigned long elapsed=TickCount()-start;char message[280];
 snprintf(message,sizeof message,"NETPROFILE rc=%d ticks=%lu checks=%lu yields=%lu pauses=%lu wait=%lu status=%lu empty=%lu recv=%lu send=%lu gap=%lu system-peak=%ld system-delta=%ld\n",result,elapsed,checks,yield_ticks,sleeps,pause_ticks,status_calls,empty_calls,recv_calls,send_calls,max_gap,system_before-system_low,system_free()-system_before);say(message);int f=cli_file_open("/tmp/4dmetrics","a");if(f>=0){cli_file_write(f,message,strlen(message));cli_file_close(f);}return result;}
