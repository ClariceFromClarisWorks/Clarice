/* Native .IPP and dnrp adapters; see outputs/network-foundation/RESEARCH.md.
 * Callbacks only write caller-owned records. Never unload pending callback code.
 */
#include <Multiverse.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "native.h"
#include "abi.h"
void NetFormatIP(unsigned long a,char text[16]){
 snprintf(text,16,"%lu.%lu.%lu.%lu",(a>>24)&255,(a>>16)&255,(a>>8)&255,a&255);
}
static OSErr driver(short *ref){return OpenDriver((ConstStringPtr)"\p.IPP",ref);}
OSErr NetAddress(unsigned long *address,unsigned long *mask){
 GetAddrParamBlock p;memset(&p,0,sizeof p);OSErr e=driver(&p.ioCRefNum);if(e)return e;
 p.csCode=ipctlGetAddr;e=PBControlSync((ParmBlkPtr)&p);
 if(!e){*address=p.ourAddress;*mask=p.ourNetMask;}return e;
}
typedef long (*Resolver)(long,...);
typedef struct {volatile short done;NetHostInfo answer;} Lookup;
static pascal void resolved(NetHostInfo *answer,char *user){(void)answer;((Lookup*)user)->done=1;}
static Handle resolver_resource(OSErr *why){
 SysEnvRec env;short saved=CurResFile(),file=-1;Handle code=0;
 *why=SysEnvirons(1,&env);if(*why)return 0;*why=resNotFound;
 static const unsigned char *names[]={
  (const unsigned char*)"\pMacTCP DNR",(const unsigned char*)"\pMacTCP",
  (const unsigned char*)"\pMoofTCP",(const unsigned char*)"\p:Control Panels:MacTCP",
  (const unsigned char*)"\p:Control Panels:MoofTCP"};
 for(unsigned i=0;i<sizeof names/sizeof names[0];i++){
  file=OpenRFPerm(names[i],env.sysVRefNum,fsRdPerm);
  if(file==-1){*why=ResError();if(*why==-108)break;continue;}
  code=Get1IndResource('dnrp',1);*why=ResError();if(code){LoadResource(code);*why=ResError();if(*why||!*code)code=0;}
  if(code){DetachResource(code);HLock(code);}
  CloseResFile(file);if(code||*why==-108)break;
 }
 UseResFile(saved);return code;
}
OSErr NetResolve(const char *name,unsigned long *address,NetPoll poll,unsigned long limit){
 unsigned long numeric=NetParseIP(name);if(numeric){*address=numeric;return 0;}
 /* Reject URL syntax/control characters before loading executable resolver code. */
 size_t n=strlen(name);if(!n||n>253)return paramErr;
 for(size_t i=0;i<n;i++){unsigned char c=name[i];if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='.'))return paramErr;}
 if(poll&&poll())return userCanceledErr;
 OSErr resource_error=0;Handle code=resolver_resource(&resource_error);if(!code)return resource_error?resource_error:resNotFound;
 Resolver call=(Resolver)*code;OSErr e=(OSErr)call(1L,(char*)0);
 if(!e){
  Lookup q;memset(&q,0,sizeof q);unsigned long start=TickCount();OSErr stop=0;
  e=(OSErr)call(3L,name,&q.answer,resolved,(char*)&q);
  if(e==cacheFault||e==1){
   while(!q.done){if(poll&&poll())stop=userCanceledErr;
    if(!stop&&(unsigned long)(TickCount()-start)>=limit)stop=commandTimeout;
    SystemTask(); /* no CloseResolver until this outstanding call completes */
   }
   e=stop?stop:q.answer.rtnCode;
  }
  if(!e){if(!q.answer.addr[0])e=noAnsErr;else *address=q.answer.addr[0];}
  OSErr closed=(OSErr)call(2L);if(!e)e=closed;
 }
 HUnlock(code);DisposeHandle(code);return e;
}
typedef struct {volatile short done;short result;unsigned long ticks,limit;unsigned char data[16];} Echo;
/* MacTCP echo callback receives its OWN parameter block in A0, not our request.
   No Toolbox, allocation or application globals are accessed at interrupt time. */
void net_echo_result(NetICMPBlock *p){
 Echo *e=(Echo*)p->icmpEchoInfo.userDataPtr;e->result=p->ioResult;
 e->ticks=(uint32_t)(p->icmpEchoInfo.echoReplyIn-p->icmpEchoInfo.echoRequestOut);
 if(!e->result){
  /* The installed stack owns wire identifiers/sequences. Verify its returned
     data as well; never count an empty/unsupported completion as a reply.
     Apple MacTCP can report a length including 8 extra bytes while its
     pointer already addresses the payload; compare only our bounded prefix.
     Volatile byte reads prevent an interrupt-time imported libc call. */
  const volatile unsigned char *data=(const volatile unsigned char*)p->icmpEchoInfo.echoedData.ptr;
  if(!data||p->icmpEchoInfo.echoedData.length<sizeof e->data||e->ticks>e->limit)e->result=NET_ECHO_INVALID;
  else for(unsigned i=0;i<sizeof e->data;i++)if(data[i]!=e->data[i]){e->result=NET_ECHO_INVALID;break;}
 }
 e->done=1;
}
extern void net_echo_callback(void);
#ifndef NETWORK_HOST_TEST
__asm__(".text\n.align 2\n.globl net_echo_callback\nnet_echo_callback:\n"
 "movem.l %d0-%d2/%a0-%a2,-(%sp)\nmove.l %a0,-(%sp)\n"
 "jsr net_echo_result\naddq.l #4,%sp\nmovem.l (%sp)+,%d0-%d2/%a0-%a2\nrts\n");
#endif
OSErr NetEcho(unsigned long address,unsigned short seconds,NetPoll poll,unsigned long *ticks){
 NetIPBlock p;Echo reply;static uint32_t sequence;
 memset(&p,0,sizeof p);memset(&reply,0,sizeof reply);OSErr e=driver(&p.ioCRefNum);if(e)return e;
 if(poll&&poll())return userCanceledErr;
 memcpy(reply.data,"Clarice!",8);uint32_t stamp=(uint32_t)TickCount(),serial=++sequence;
 for(unsigned i=0;i<4;i++){reply.data[8+i]=(unsigned char)(stamp>>(24-i*8));reply.data[12+i]=(unsigned char)(serial>>(24-i*8));}
 /* Allow a one-second scheduling margin, not backwards/uninitialised times. */
 reply.limit=((unsigned long)seconds+1)*61;
 p.csCode=17;p.csParam.IPEchoPB.dest=address;
 p.csParam.IPEchoPB.data.length=sizeof reply.data;p.csParam.IPEchoPB.data.ptr=(Ptr)reply.data;
 p.csParam.IPEchoPB.timeout=seconds;p.csParam.IPEchoPB.icmpCompletion=(ProcPtr)net_echo_callback;
 p.csParam.IPEchoPB.userDataPtr=(unsigned long)&reply;
 /* Submission is asynchronous too: keep PB alive until ioResult is final. */
 e=PBControlAsync((ParmBlkPtr)&p);int cancelled=0;
 if(e<0)return e;
 while(*(volatile short*)&p.ioResult>0){if(poll&&poll())cancelled=1;SystemTask();}
 e=p.ioResult;if(e)return cancelled?userCanceledErr:e;
 while(!reply.done){if(poll&&poll())cancelled=1;SystemTask();}
 if(cancelled)return userCanceledErr;
 if(!reply.result)*ticks=reply.ticks;return reply.result;
}
