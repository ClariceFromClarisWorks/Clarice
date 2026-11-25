/* Execute production client with fault-injected Device Manager calls. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define SERIAL_HOST_TEST 1
#define baud4800 22
#define baud9600 10
#define baud19200 4
#define baud38400 1
#define baud57600 0
#define data8 3072
#define noParity 0
#define stop10 16384
#define ioErr -36
#define userCanceledErr -128
typedef struct {unsigned char fXOn,fCTS,xOn,xOff,errs,evts,fInX,null;} SerShk;
typedef struct {unsigned char cumErrs;} SerStaRec;
typedef struct {struct {short ioRefNum,ioResult;char *ioBuffer;long ioReqCount,ioActCount;} ioParam;} ParamBlockRec;
static int buffer_live,kill_fail,restore_fail,close_fail,delayed_kill;
static int step,fail_call,calls,cancel_at,stall,overrun,overflow,short_write,tty=1;
static int opened,closed,killed,restored,active,interactive,configured,selected,reads_while_pending;
static unsigned long ticks;
static ParamBlockRec *pending;
static unsigned char keys[8192],sent[8192],received[8192];
static int keylen,keyat,sentlen,rxlen,rxat;
static char display[16384];static int displaylen;
static int fault(void){return ++calls==fail_call?-36:0;}
static int app_yield(void){step++;ticks++;assert(step<2000);if(pending&&!stall&&step%3==0){memcpy(sent+sentlen,pending->ioParam.ioBuffer,pending->ioParam.ioReqCount);sentlen+=pending->ioParam.ioReqCount;pending->ioParam.ioActCount=pending->ioParam.ioReqCount-(short_write?1:0);pending->ioParam.ioResult=0;pending=0;}return cancel_at&&step>=cancel_at;}
static int app_activity_interactive(int n){interactive=n;return 0;}
unsigned long cli_ticks(void){return ticks;}
int cli_isatty(int n){(void)n;return tty;}
int app_serial_tty(int n){active=n;return 0;}
int app_serial_key(void){if(overflow)return -2;return keyat<keylen?keys[keyat++]:-1;}
long cli_write(int fd,const void *p,unsigned long n){(void)fd;assert(displaylen+n<sizeof display);memcpy(display+displaylen,p,n);displaylen+=n;display[displaylen]=0;return n;}
static int OpenDriver(const unsigned char *s,short *r){int e=fault();if(e)return e;selected=s[2]=='B';*r=++opened;return 0;}
static int CloseDriver(short r){if(close_fail){close_fail--;return -36;}assert(r&&!buffer_live&&!pending);closed++;return 0;}
static int KillIO(short r){assert(r);if(kill_fail){kill_fail--;return -36;}if(delayed_kill){delayed_kill--;return 0;}if(pending&&pending->ioParam.ioRefNum==r){pending->ioParam.ioResult=-27;pending=0;}killed++;return 0;}
static int SerReset(short r,int config){(void)r;configured=config;return fault();}
static int SerHShake(short r,const SerShk *s){(void)r;assert(!s->fCTS&&!s->fXOn&&!s->fInX);return fault();}
static int SerSetBuf(short r,char *b,int n){(void)r;if(!n){if(restore_fail){restore_fail--;return -36;}assert(!b);buffer_live=0;restored++;return 0;}assert(n==4096);int e=fault();if(!e)buffer_live=1;return e;}
static int SerStatus(short r,SerStaRec *s){(void)r;s->cumErrs=overrun;return fault();}
static int SerGetBuf(short r,long *n){(void)r;*n=rxlen-rxat;return fault();}
static int FSRead(short r,long *n,char *b){(void)r;if(pending)reads_while_pending++;memcpy(b,received+rxat,*n);rxat+=*n;return fault();}
static int PBWriteAsync(ParamBlockRec *p){int e=fault();if(e)return e;assert(!pending);p->ioParam.ioResult=1;pending=p;return 0;}
static void SystemTask(void){}
static void Delay(long n,long *out){ticks+=n;*out=ticks;}
#include "../../src/commands/serial.c"
static void reset(void){kill_fail=restore_fail=close_fail=delayed_kill=0;buffer_live=step=fail_call=calls=cancel_at=stall=overrun=overflow=short_write=0;opened=closed=killed=restored=active=interactive=configured=selected=reads_while_pending=0;ticks=0;pending=0;keylen=keyat=sentlen=rxlen=rxat=displaylen=0;display[0]=0;tty=1;}
static int run(char *device){char *v[]={"serial","-b","57600",device};int r=clarice_main(4,v);assert(!pending&&!active&&!interactive);assert(opened==closed);return r;}
int main(void){
 reset();for(int i=0;i<4096;i++)keys[keylen++]='a'+i%26;keys[keylen++]=29;keys[keylen++]='q';
 for(int i=0;i<2048;i++)received[rxlen++]='A'+i%26;
 assert(!run("/dev/tty.modem"));assert(sentlen==4096&&!memcmp(sent,keys,4096));assert(rxat==rxlen&&reads_while_pending>0);assert(configured==(baud57600|data8|stop10)&&restored==1&&!selected);
 reset();keys[keylen++]=29;keys[keylen++]=29;keys[keylen++]=29;keys[keylen++]='x';keys[keylen++]=29;keys[keylen++]='q';assert(!run("/dev/tty.printer"));assert(selected&&sentlen==3&&sent[0]==29&&sent[1]==29&&sent[2]=='x');
 for(int f=1;f<=12;f++){reset();fail_call=f;keys[keylen++]='z';cancel_at=20;assert(run("/dev/tty.modem")!=0);}
 reset();cancel_at=5;keys[keylen++]='x';stall=1;assert(run("/dev/tty.modem")==130);assert(killed==1);
 reset();keys[keylen++]='x';stall=1;assert(run("/dev/tty.modem")==1);assert(strstr(display,"transmit timeout"));
 reset();overflow=1;assert(run("/dev/tty.modem")==1);assert(!sentlen&&strstr(display,"overflow"));
 reset();overrun=32;assert(run("/dev/tty.modem")==1);assert(strstr(display,"data lost"));
 reset();short_write=1;keys[keylen++]='x';assert(run("/dev/tty.modem")==1);assert(strstr(display,"short serial write"));
 reset();assert(run("/dev/tty.invalid")==2);assert(!opened);
 reset();tty=0;assert(run("/dev/tty.modem")==2);assert(!opened);
 reset();char *probe[]={"serial","modem","9600","--test"};memcpy(received,"CLARICE-PONG",12);rxlen=12;assert(!clarice_main(4,probe));assert(strstr(display,"SERIAL_TEST PASS"));
 reset();assert(clarice_main(4,probe)==1);assert(strstr(display,"probe timeout"));
 for(int f=0;f<4;f++){reset();cancel_at=5;keys[keylen++]='x';stall=1;
  if(f==0)kill_fail=3;else if(f==1)restore_fail=3;else if(f==2)close_fail=3;else delayed_kill=3;
  int rc=run("/dev/tty.modem");assert(rc==1||rc==130);assert(!pending&&!buffer_live&&!kill_fail&&!restore_fail&&!close_fail&&!delayed_kill);
 }
 puts("PASS: 4096 TX / 2048 RX ordered concurrent bytes, both ports, escapes, 12 faults, cancel, timeouts, overflow, short write, bad path/TTY, probe, cleanup");
}
