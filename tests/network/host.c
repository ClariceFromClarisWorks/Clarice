#define NETWORK_HOST_TEST
#include "../../src/commands/network/native.c"
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
static int handles,closed,pending,mode,step,cancel_at,polls,missing,stack_error;
static unsigned long now;
static Lookup *lookup;static NetIPBlock *echo;
static long resolver(long op,...){
 va_list ap;va_start(ap,op);long result=0;
 if(op==1){if(mode==5)result=-23044;}
 else if(op==2){assert(!pending);closed++;}
 else {assert(op==3);(void)va_arg(ap,const char*);NetHostInfo *h=va_arg(ap,NetHostInfo*);(void)va_arg(ap,void*);lookup=(Lookup*)va_arg(ap,char*);
  if(mode==1){h->addr[0]=0x01020304;result=0;}else if(mode==2)result=-23045;else{pending=1;result=cacheFault;}}
 va_end(ap);return result;
}
short CurResFile(void){return 9;}short SysEnvirons(short v,SysEnvRec *e){(void)v;e->sysVRefNum=1;return 0;}
short OpenRFPerm(ConstStringPtr s,short v,char p){(void)s;(void)v;assert(p==fsRdPerm);return missing?-1:10;}
Handle Get1IndResource(unsigned long t,short n){(void)t;assert(n==1);if(mode==6)return 0;Handle h=malloc(sizeof *h);assert(h);*h=(char*)resolver;handles++;return h;}
void LoadResource(Handle h){assert(h);}void DetachResource(Handle h){assert(h);}void HLock(Handle h){assert(h);}void HUnlock(Handle h){assert(h);}
void DisposeHandle(Handle h){assert(!pending);free(h);handles--;}
void CloseResFile(short f){assert(f==10);}void UseResFile(short f){assert(f==9);}short ResError(void){return mode==6?-108:0;}
short OpenDriver(ConstStringPtr s,short *r){(void)s;*r=-10;return stack_error;}
short PBControlSync(ParmBlkPtr b){GetAddrParamBlock *p=b;assert(p->csCode==15);p->ourAddress=0x01020304;p->ourNetMask=0xffffff00;return 0;}
short PBControlAsync(ParmBlkPtr b){echo=b;pending=2;echo->ioResult=1;return 0;}
void net_echo_callback(void){}
unsigned long TickCount(void){return now;}
int poll(void){polls++;return cancel_at&&polls==cancel_at;}
void SystemTask(void){now++;if(++step<4)return;
 if(pending==1){lookup->answer.addr[0]=0x01020304;lookup->answer.rtnCode=mode==3?-23046:0;resolved(&lookup->answer,(char*)lookup);pending=0;}
 else if(pending==2){echo->ioResult=0;pending=3;}
 else if(pending==3){NetICMPBlock b;memset(&b,0,sizeof b);b.icmpEchoInfo.userDataPtr=echo->csParam.IPEchoPB.userDataPtr;b.icmpEchoInfo.echoRequestOut=1;b.icmpEchoInfo.echoReplyIn=3;b.ioResult=mode==3?-23035:0;net_echo_result(&b);pending=0;}
}
unsigned long NetParseIP(const char *s){return !strcmp(s,"1.2.3.4")?0x01020304:0;}
static void reset(void){assert(!handles&&!pending);mode=step=cancel_at=polls=missing=closed=stack_error=0;now=0;}
int main(void){unsigned long a=0,m,t;
 reset();assert(!NetAddress(&a,&m)&&a==0x01020304&&m==0xffffff00);
 stack_error=-43;assert(NetAddress(&a,&m)==-43);
 reset();assert(!NetResolve("1.2.3.4",&a,poll,30)&&!closed&&!handles);
 for(int i=0;i<6;i++){reset();mode=i;short e=NetResolve("example.test",&a,poll,30);assert(e==(i==2?-23045:i==3?-23046:i==5?-23044:0));assert(!handles&&!pending);}
 reset();mode=6;assert(NetResolve("example.test",&a,poll,30)==-108&&!handles);
 reset();missing=1;assert(NetResolve("example.test",&a,poll,30)==resNotFound);
 reset();cancel_at=3;assert(NetResolve("example.test",&a,poll,30)==userCanceledErr&&closed==1&&!pending);
 reset();assert(NetResolve("example.test",&a,poll,1)==commandTimeout&&closed==1&&!pending);
 reset();assert(NetResolve("bad@name",&a,poll,30)==paramErr&&!handles);
 for(int i=0;i<3;i++){reset();if(i==1)cancel_at=3;if(i==2)mode=3;assert(NetEcho(0x01020304,2,poll,&t)==(i==1?userCanceledErr:i==2?-23035:0));assert(!pending&&!handles);if(!i)assert(t==2);}
 reset();assert(!NetResolve("example.test",&a,poll,30));assert(!NetEcho(a,2,poll,&t));
 puts("PASS native adapter: status, numeric, immediate/deferred DNS, DNS error/missing/open failure, cancellation/deadline drain, ICMP submit/reply/timeout/cancel, repeated cleanup");
}
