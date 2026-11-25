/* Actual shared transport, not a replacement NetAvailable mock. */
#include "../../src/commands/vendor/mactcp/nettcp.c"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
static short status_error,receive_error,release_error,close_error;static int state=8;static unsigned unread,allocations;
short OpenDriver(ConstStringPtr n,short *r){(void)n;*r=1;return 0;}
Ptr NewPtrSys(unsigned long n){allocations++;return malloc(n);}void DisposePtr(Ptr p){assert(allocations);allocations--;free(p);}
short MemError(void){return -108;}
short PBControlSync(ParmBlkPtr b){TCPiopb *p=b;
 switch(p->csCode){
 case TCPCreate:p->tcpStream=(Ptr)1;return 0;
 case TCPStatus:if(status_error)return status_error;p->csParam.status.connectionState=state;p->csParam.status.amtUnreadData=unread;return 0;
 case TCPRcv:return receive_error;
 case TCPClose:return close_error;
 case TCPRelease:return release_error;
 default:return 0;
 }
}
int main(void){NetConn c={0};char data[10];
 assert(NetAvailable(&c)==connectionDoesntExist);
 for(int i=0;i<20;i++){
  assert(!NetConnect(&c,1,21)&&allocations==1);state=8;unread=0;assert(!NetAvailable(&c));
  unread=10;assert(NetAvailable(&c)==10);state=14;assert(NetAvailable(&c)==10);
  unread=0;assert(NetAvailable(&c)==connectionClosing);
  state=0;assert(NetAvailable(&c)==connectionTerminated);
  short errors[]={-1,-36,commandTimeout,connectionTerminated,connectionDoesntExist,invalidStreamPtr};
  for(unsigned j=0;j<sizeof errors/sizeof *errors;j++){status_error=errors[j];assert(NetAvailable(&c)==errors[j]);receive_error=errors[j];assert(NetRecv(&c,data,10,1)==errors[j]);}
  status_error=0;receive_error=connectionClosing;assert(NetRecv(&c,data,10,1)==connectionClosing);
  receive_error=0;assert(NetRecv(&c,data,10,1)==10);release_error=-36;close_error=-23012;assert(NetClose(&c)==-36&&allocations==1&&c.stream&&c.rcvBuff);
  release_error=close_error=0;assert(!NetClose(&c)&&!allocations);assert(!NetClose(&c)&&!allocations);
 }
 puts("PASS TCP: queued FIN data, orderly EOF, reset/invalid state, original status and receive errors, failed native teardown retains system buffer; retry/idempotent close; 20 system-buffer allocation/release cycles");
}
