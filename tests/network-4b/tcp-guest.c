/* Disposable probe: real .IPP operations; only release/abort failures and one
 * cancellation signal are injected. Never part of the distribution. */
#include <Multiverse.h>
#include <Memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int app_yield(void),cli_pause_ticks(unsigned long);
extern void say(const char *);
static unsigned waits,abort_fail,release_fail,cancel_after;
static unsigned long last,maxgap;
static OSErr control(ParmBlkPtr b){TCPiopb *p=(TCPiopb*)b;
 if(p->csCode==TCPRelease&&release_fail){release_fail--;return -36;}
 if(p->csCode==TCPAbort&&abort_fail){abort_fail--;return -36;}
 return PBControlSync(b);
}
static int pause_one(unsigned long n){unsigned long now=TickCount();unsigned long gap=now-last;if(last&&gap>maxgap)maxgap=gap;last=now;waits++;int c=cli_pause_ticks(n);return c||(cancel_after&&waits>=cancel_after);}
#define PBControlSync control
#define cli_pause_ticks pause_one
#include "../../src/commands/vendor/mactcp/nettcp.c"
#undef cli_pause_ticks
#undef PBControlSync
static long sysfree(void){THz old=GetZone();SetZone(LMGetSysZone());long n=FreeMem();SetZone(old);return n;}
int clarice_main(int argc,char **argv){if(argc!=4)return 2;NetConn c={0},second={0};char b[160],rx[32];long before=sysfree();unsigned long start=TickCount();
 OSErr e=NetConnect(&c,NetParseIP(argv[1]),atoi(argv[2]));int mode=atoi(argv[3]);long low=sysfree();
 if(!e&&mode==3){e=NetConnect(&second,NetParseIP(argv[1]),atoi(argv[2]));low=sysfree();}
 if(!e){
  if(mode==1){release_fail=3;}
  if(mode==2){abort_fail=3;cancel_after=waits+8;}
  e=NetSend(&c,"probe\n",6);
  if(!e){long n=NetRecv(&c,rx,sizeof rx,5);if(n<0)e=n;}
 }
 (void)NetClose(&second);OSErr close=NetClose(&c);long after=sysfree();
 snprintf(b,sizeof b,"TCP probe mode=%d result=%d close=%d waits=%u maxgap=%lu ticks=%lu system-delta=%ld peak-system=%ld owned=%d\n",mode,e,close,waits,maxgap,TickCount()-start,after-before,before-low,!!(c.stream||c.rcvBuff||second.stream||second.rcvBuff));say(b);
 return e==userCanceledErr||close==userCanceledErr?130:e||close?1:0;
}
