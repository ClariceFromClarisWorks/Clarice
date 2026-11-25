/* Exercise the actual foreground transport, including asynchronous ownership. */
#include "../../src/commands/vendor/mactcp/nettcp.c"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
static short status_error,receive_error,open_error,send_error,close_error,submit_error;
static int state=8,release_fail,abort_fail,delay,remaining,cancel_at,hang,create_fail,alloc_fail;
static unsigned unread,allocations,peak,pauses,aborts,releases,active;
static unsigned long ticks;
static TCPiopb *pending;
short OpenDriver(ConstStringPtr n,short *r){(void)n;*r=1;return 0;}
Ptr NewPtrSys(unsigned long n){assert(n==16384);if(alloc_fail)return 0;allocations++;if(allocations>peak)peak=allocations;return malloc(n);}
void DisposePtr(Ptr p){assert(allocations&&!pending);allocations--;free(p);}
short MemError(void){return -108;}
unsigned long TickCount(void){return ticks;}
int app_yield(void){return cancel_at&&pauses>=(unsigned)cancel_at;}
long cli_write(int f,const void *p,unsigned long n){(void)f;(void)p;return n;}
static short completion(short code){switch(code){case TCPActiveOpen:return open_error;case TCPSend:return send_error;case TCPRcv:return receive_error;case TCPClose:return close_error;}abort();}
int cli_pause_ticks(unsigned long n){assert(n==1);ticks++;pauses++;
 if(pending){
  /* All referenced stack and command bytes must still be addressable. ASan
     observes these reads on every simulated driver progress step. */
  if(pending->csCode==TCPSend){struct W{unsigned short length;Ptr ptr;unsigned short term;};struct W *w=(void*)pending->csParam.send.wdsPtr;assert(w->length&&w->ptr&&!w->term);volatile char byte=w->ptr[w->length-1];(void)byte;}
  if(pending->csCode==TCPRcv){assert(pending->csParam.receive.rcvBuffLen);pending->csParam.receive.rcvBuff[0]='x';}
  if(!hang&&!--remaining){pending->ioResult=completion(pending->csCode);pending=0;}
 }
 assert(pauses<10000);return app_yield();
}
short PBControlSync(ParmBlkPtr b){TCPiopb *p=b;
 switch(p->csCode){
 case TCPCreate:if(create_fail)return -36;active++;p->tcpStream=(Ptr)(size_t)active;return 0;
 case TCPStatus:if(status_error)return status_error;p->csParam.status.connectionState=state;p->csParam.status.amtUnreadData=unread;return 0;
 case TCPAbort:aborts++;if(abort_fail){abort_fail--;return -36;}if(pending){pending->ioResult=connectionTerminated;pending=0;}return 0;
 case TCPRelease:releases++;assert(!pending);if(release_fail){release_fail--;return -36;}assert(active);active--;return 0;
 default:assert(!"peer-dependent operation submitted synchronously");return -1;
 }
}
short PBControlAsync(ParmBlkPtr b){TCPiopb *p=b;assert(!pending);if(submit_error)return submit_error;p->ioResult=1;pending=p;remaining=delay?delay:1;return 0;}
static void reset(void){assert(!pending&&!active&&!allocations);status_error=receive_error=open_error=send_error=close_error=submit_error=0;state=8;release_fail=abort_fail=delay=cancel_at=hang=create_fail=alloc_fail=0;unread=pauses=aborts=releases=0;ticks=0;}
int main(void){NetConn c={0},d={0};char data[10]={0};
 assert(NetAvailable(&c)==connectionDoesntExist);
 for(int i=0;i<20;i++){
  reset();delay=3;assert(!NetConnect(&c,1,21)&&allocations==1&&pauses==3);
  assert(NetConnect(&c,1,21)==paramErr&&allocations==1);
  assert(!NetAvailable(&c));unread=10;assert(NetAvailable(&c)==10);state=14;assert(NetAvailable(&c)==10);
  unread=0;assert(NetAvailable(&c)==connectionClosing);state=0;assert(NetAvailable(&c)==connectionTerminated);
  short errors[]={-1,-36,commandTimeout,connectionTerminated,connectionDoesntExist,invalidStreamPtr};
  for(unsigned j=0;j<sizeof errors/sizeof *errors;j++){status_error=errors[j];assert(NetAvailable(&c)==errors[j]);receive_error=errors[j];assert(NetRecv(&c,data,10,1)==errors[j]);}
  status_error=0;receive_error=connectionClosing;assert(NetRecv(&c,data,10,1)==connectionClosing);
  receive_error=0;assert(NetRecv(&c,data,10,1)==10);assert(!NetSend(&c,data,10));
  release_fail=3;close_error=-23012;assert(NetClose(&c)==-23012&&!allocations&&!c.stream&&!c.rcvBuff&&!c.rcvBuffLen);assert(releases==4);assert(!NetClose(&c));
 }
 reset();alloc_fail=1;assert(NetConnect(&c,1,21)==-108&&!allocations);reset();create_fail=1;assert(NetConnect(&c,1,21)==-36&&!allocations);
 reset();open_error=openFailed;release_fail=3;assert(NetConnect(&c,1,21)==openFailed&&!allocations&&releases==4);
 reset();submit_error=-36;assert(NetConnect(&c,1,21)==-36&&!allocations);
 for(int op=0;op<4;op++){
  reset();if(op)assert(!NetConnect(&c,1,21));hang=1;abort_fail=3;release_fail=2;cancel_at=pauses+2;
  int e=op==0?NetConnect(&c,1,21):op==1?NetSend(&c,data,10):op==2?NetRecv(&c,data,10,1):NetClose(&c);
  assert(e==userCanceledErr&&!pending&&aborts>=4);
  if(op&&op!=3)assert(NetClose(&c)==userCanceledErr);assert(!active&&!allocations);assert(!NetClose(&c));
 }
 reset();hang=1;abort_fail=2;assert(NetConnect(&c,1,21)==commandTimeout&&!allocations&&ticks>=300&&ticks<330);
 reset();assert(!NetConnect(&c,1,21));hang=1;assert(NetSend(&c,data,10)==commandTimeout);hang=0;assert(!NetClose(&c));
 reset();assert(!NetConnect(&c,1,21));hang=1;assert(NetRecv(&c,data,10,1)==commandTimeout);hang=0;assert(!NetClose(&c));
 reset();assert(!NetConnect(&c,1,21));hang=1;assert(NetClose(&c)==commandTimeout&&!allocations);
 reset();assert(!NetConnect(&c,1,21)&&!NetConnect(&d,1,20)&&allocations==2);release_fail=5;assert(NetClose(&d)==-36&&allocations==1);assert(!NetClose(&c)&&!allocations);assert(peak==2);
 puts("PASS actual TCP: async open/send/receive/close, delayed completion, refused/failed submit, timeout, cancellation with failed abort, failed release/retry, preserved native errors/EOF, idempotent cleanup; 20 repeated cycles; peak two 16 KiB system buffers, zero retained ownership");
}
