/* Deterministic driver ownership/failure tests, not a MacTCP emulator. */
#include "Multiverse.h"
#include "../vendor/nettcp.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static int live, allocations,release_failure,create_failure,alloc_failure,open_error,pending_forever,cancel,cancel_pending,aborts,polls;
static unsigned long ticks;static TCPiopb *pending;static Ptr owned;
OSErr OpenDriver(ConstStringPtr s,short*r){(void)s;*r=-1;return 0;}
OSErr MemError(void){return -108;}
Ptr NewPtrSys(unsigned long n){if(alloc_failure)return NULL;live++;allocations++;return malloc(n);}
void DisposePtr(Ptr p){assert(p!=owned);assert(live>0);live--;free(p);}
unsigned long TickCount(void){return ticks;}
void SystemTask(void){}
OSErr PBControlSync(TCPiopb*p){
 switch(p->csCode){
 case TCPCreate:if(create_failure)return -108;assert(!owned);owned=p->csParam.create.rcvBuff;p->tcpStream=(Ptr)1;break;
 case TCPAbort:aborts++;if(pending){pending->ioResult=connectionTerminated;pending=NULL;}break;
 case TCPRelease:if(release_failure)return -50;owned=NULL;break;
 default:assert(0);
 }return 0;
}
OSErr PBControlAsync(TCPiopb*p){assert(p->csCode==TCPActiveOpen);assert(!pending);pending=p;p->ioResult=1;return 0;}
static int poll_events(void){polls++;ticks+=6;if(pending&&!pending_forever){pending->ioResult=open_error;pending=NULL;}return cancel||(cancel_pending&&pending!=NULL);}
int main(void){NetConn c={0};
 for(int i=0;i<30;i++){assert(!NetConnect(&c,1,1,poll_events));assert(c.isOpen&&live==1);assert(!NetClose(&c));assert(!live&&!owned&&!c.stream);}
 alloc_failure=1;assert(NetConnect(&c,1,1,poll_events)==-108);assert(!live);alloc_failure=0;
 create_failure=1;assert(NetConnect(&c,1,1,poll_events)==-108);assert(!live);create_failure=0;
 open_error=openFailed;assert(NetConnect(&c,1,1,poll_events)==openFailed);assert(!live);open_error=0;
 pending_forever=1;int old=aborts;assert(NetConnect(&c,1,1,poll_events)==commandTimeout);assert(aborts>old&&!live&&!pending);pending_forever=0;
 assert(!NetConnect(&c,1,1,poll_events));release_failure=1;Ptr saved=c.rcvBuff;int count=allocations;
 assert(NetClose(&c)==-50);assert(live==1&&c.rcvBuff==saved&&owned==saved);
 for(int i=0;i<5;i++)assert(NetConnect(&c,1,1,poll_events)==-50);
 assert(allocations==count&&live==1);release_failure=0;assert(!NetConnect(&c,1,1,poll_events));assert(!NetClose(&c));
 cancel=1;assert(NetConnect(&c,1,1,poll_events)==-128);assert(!live);cancel=0;
 pending_forever=cancel_pending=1;assert(NetConnect(&c,1,1,poll_events)==-128);assert(!live&&!pending);pending_forever=cancel_pending=0;
 assert(!NetConnect(&c,1,1,poll_events));assert(!NetClose(&c));assert(!live&&!pending);assert(polls>30);
 puts("PASS: 30 cycles, allocation/create/open failures, timeout, retained-release ownership, retry, cancellation, recovery");return 0;}
