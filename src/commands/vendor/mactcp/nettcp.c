/*
 * nettcp.c — cooperative MacTCP client. See nettcp.h.
 */
#include <Devices.h>
#include <Memory.h>
#include <Multiverse.h> /* pinned Multiversal includes MacTCP declarations */
#include "nettcp.h"
#include <string.h>
#define IPP_RCVBUF_LEN 16384UL /* Existing system-heap receive ring per stream. */
static short gIPPRefNum = 0;
static Boolean gDriverOpen = false;
extern int app_yield(void);
extern int cli_pause_ticks(unsigned long);
extern long cli_write(int,const void *,unsigned long);

/* Abort/Release do not wait for a peer. They need their own PB while another
 * operation is outstanding; notify/completion callbacks are deliberately nil. */
static OSErr net_control(NetConn *c,short code)
{
 TCPiopb pb;memset(&pb,0,sizeof pb);pb.ioCRefNum=gIPPRefNum;
 pb.tcpStream=c->stream;pb.csCode=code;return PBControlSync((ParmBlkPtr)&pb);
}
static void net_cleanup_pause(void)
{
 /* Cancellation must not turn failed-driver cleanup into a busy spin. */
 for(unsigned i=0;i<6;i++)(void)cli_pause_ticks(1);
}
static OSErr net_wait(NetConn *c,TCPiopb *pb,unsigned long timeout)
{
 OSErr reason=0,e;unsigned long start=TickCount();
 if(app_yield())return userCanceledErr;
 e=PBControlAsync((ParmBlkPtr)pb);if(e<0)return e;
 while(*(volatile short*)&pb->ioResult>0){
  if(cli_pause_ticks(1))reason=userCanceledErr;
  if(!reason&&timeout&&TickCount()-start>=timeout)reason=commandTimeout;
  if(reason&&*(volatile short*)&pb->ioResult>0){
   (void)net_control(c,TCPAbort);
   /* Abort failure is not detachment. Keep PB, WDS and caller buffers alive
    * until the ORIGINAL operation completes, retrying cooperatively. */
   if(*(volatile short*)&pb->ioResult>0)net_cleanup_pause();
  }
 }
 return reason?reason:pb->ioResult;
}
static OSErr net_release(NetConn *c)
{
 OSErr first=0,e;
 while((e=net_control(c,TCPRelease))!=noErr){
  if(!first){const char *m="TCP cleanup pending; waiting for the native driver to release its buffer.\n";first=e;cli_write(2,m,strlen(m));}
  (void)net_control(c,TCPAbort);net_cleanup_pause();
 }
 c->stream=0;return first;
}

OSErr NetInit(void)
{
	OSErr err;
	if (gDriverOpen)
		return noErr;
	err = OpenDriver((ConstStringPtr)"\p.IPP", &gIPPRefNum);
	if (err == noErr)
		gDriverOpen = true;
	return err;
}

unsigned long NetParseIP(const char *dotted)
{
	unsigned long ip = 0;
	int part = 0, ndigits = 0, octets = 0;
	const char *p = dotted;
	for (;; p++) {
		if (*p >= '0' && *p <= '9') {
			part = part * 10 + (*p - '0');
			ndigits++;
			if (part > 255 || ndigits > 3) return 0;
		} else if (*p == '.' || *p == '\0') {
			if (ndigits == 0) return 0;
			ip = (ip << 8) | (unsigned long)part;
			octets++;
			part = 0; ndigits = 0;
			if (*p == '\0') break;
		} else {
			return 0;
		}
	}
	if (octets != 4) return 0;
	return ip;
}

OSErr NetConnect(NetConn *c, unsigned long host, unsigned short port)
{
	TCPiopb pb;
	memset(&pb, 0, sizeof pb);
	OSErr err;

	if(c->stream || c->rcvBuff)return paramErr;
	c->stream = 0;
	c->rcvBuff = nil;
	c->rcvBuffLen = 0;
	c->isOpen = 0;

	err = NetInit();
	if (err != noErr)
		return err;

	/* Allocate the MacTCP receive buffer in the SYSTEM heap, not the app heap.
	 * MacTCP holds this pointer for the stream's whole life. If the app quits
	 * cleanly, NetClose releases the stream and frees this. But if the app
	 * CRASHES (address-error bomb), NetClose never runs — MacTCP keeps a leaked
	 * stream still pointing here. An app-heap buffer gets reclaimed and reused
	 * when the dead app's heap is torn down, so MacTCP then scribbles arriving
	 * packets into reused memory and corrupts whatever app runs next → every
	 * later network app fails until reboot ("first app works, next doesn't").
	 * A system-heap buffer survives the crash: MacTCP writes to still-valid
	 * (leaked) memory, no corruption, and the next app connects fine. */
	c->rcvBuff = NewPtrSys(IPP_RCVBUF_LEN);
	if (c->rcvBuff == nil)
		return MemError();
	c->rcvBuffLen = IPP_RCVBUF_LEN;

	/* TCPCreate — register the receive buffer, get a stream */
	pb.ioCRefNum = gIPPRefNum;
	pb.csCode = TCPCreate;
	pb.csParam.create.rcvBuff = c->rcvBuff;
	pb.csParam.create.rcvBuffLen = c->rcvBuffLen;
	pb.csParam.create.notifyProc = nil;
	pb.csParam.create.userDataPtr = nil;
	err = PBControlSync((ParmBlkPtr)&pb);
	if (err != noErr) {
		DisposePtr(c->rcvBuff);
		c->rcvBuff = nil;
		c->rcvBuffLen = 0;
		return err;
	}
	c->stream = pb.tcpStream;

	/* TCPActiveOpen — connect to host:port */
	pb.ioCRefNum = gIPPRefNum;
	pb.csCode = TCPActiveOpen;
	pb.tcpStream = c->stream;
	pb.csParam.open.ulpTimeoutValue = 5;	/* seconds */
	pb.csParam.open.ulpTimeoutAction = 1;	/* abort on timeout */
	pb.csParam.open.validityFlags = 0xC0;	/* timeoutValue + timeoutAction valid */
	pb.csParam.open.commandTimeoutValue = 5;
	pb.csParam.open.remoteHost = host;
	pb.csParam.open.remotePort = port;
	pb.csParam.open.localHost = 0;
	pb.csParam.open.localPort = 0;
	pb.csParam.open.tosFlags = 0;
	pb.csParam.open.precedence = 0;
	pb.csParam.open.dontFrag = 0;
	pb.csParam.open.timeToLive = 0;
	pb.csParam.open.security = 0;
	pb.csParam.open.optionCnt = 0;
	pb.csParam.open.userDataPtr = nil;
	err = net_wait(c,&pb,300);
 if(err){(void)NetClose(c);return err;}

	c->isOpen = 1;
	return noErr;
}

long NetAvailable(NetConn *c)
{
	TCPiopb pb;
	memset(&pb, 0, sizeof pb);
	OSErr err;

	if (!c->isOpen)
		return connectionDoesntExist;

	pb.ioCRefNum = gIPPRefNum;
	pb.csCode = TCPStatus;
	pb.tcpStream = c->stream;
	pb.csParam.status.userDataPtr = nil;
	err = PBControlSync((ParmBlkPtr)&pb);
	if (err != noErr)
		return err;
	/* Drain queued data, then recognize peer FIN/closed states. MacTCP
	 * Programmer's Guide TCPStatus: ESTABLISHED = 8. This active-only
	 * client has already completed TCPActiveOpen before polling here. */
	if (!pb.csParam.status.amtUnreadData && pb.csParam.status.connectionState != 8)
		/* Only CLOSE-WAIT (peer FIN) establishes orderly EOF for an active
		 * client that has not issued TCPClose. CLOSED/reset is not success. */
		return pb.csParam.status.connectionState == 14 ? connectionClosing : connectionTerminated;
	return (long)pb.csParam.status.amtUnreadData;
}

long NetRecv(NetConn *c, void *buf, unsigned short maxlen, unsigned char timeoutSecs)
{
	TCPiopb pb;
	memset(&pb, 0, sizeof pb);
	OSErr err;

	if (!c->isOpen)
		return connectionDoesntExist;

	pb.ioCRefNum = gIPPRefNum;
	pb.csCode = TCPRcv;
	pb.tcpStream = c->stream;
	pb.csParam.receive.commandTimeoutValue = (SInt8)timeoutSecs;
	pb.csParam.receive.rcvBuff = (Ptr)buf;
	pb.csParam.receive.rcvBuffLen = maxlen;
	pb.csParam.receive.userDataPtr = nil;
	err = net_wait(c,&pb,(unsigned long)timeoutSecs*60);

	if (err == noErr)
		return (long)pb.csParam.receive.rcvBuffLen;	/* bytes actually received */

	return (long)err; /* OSErr is already negative. */
}

OSErr NetSend(NetConn *c, const void *buf, unsigned short len)
{
	TCPiopb pb;
	memset(&pb, 0, sizeof pb);
	struct { unsigned short length; Ptr ptr; unsigned short term; } wds;

	if (!c->isOpen)
		return connectionDoesntExist;

	/* one-entry write-data structure, zero-terminated */
	wds.length = len;
	wds.ptr = (Ptr)buf;
	wds.term = 0;

	pb.ioCRefNum = gIPPRefNum;
	pb.csCode = TCPSend;
	pb.tcpStream = c->stream;
	pb.csParam.send.ulpTimeoutValue = 5;
	pb.csParam.send.ulpTimeoutAction = 1;
	pb.csParam.send.validityFlags = 0xC0;
	pb.csParam.send.pushFlag = 1;
	pb.csParam.send.urgentFlag = 0;
	pb.csParam.send.wdsPtr = (Ptr)&wds;
	pb.csParam.send.userDataPtr = nil;
	return net_wait(c,&pb,300);
}

OSErr NetClose(NetConn *c)
{
 OSErr e=0;
 if(c->stream){
  if(c->isOpen){
   TCPiopb pb;memset(&pb,0,sizeof pb);pb.ioCRefNum=gIPPRefNum;
   pb.csCode=TCPClose;pb.tcpStream=c->stream;
   pb.csParam.close.ulpTimeoutValue=5;pb.csParam.close.ulpTimeoutAction=1;
   pb.csParam.close.validityFlags=0xC0;
   e=net_wait(c,&pb,300);
  }
  OSErr release=net_release(c);if(!e)e=release;
 }
 if(c->rcvBuff){DisposePtr(c->rcvBuff);c->rcvBuff=nil;}
 c->rcvBuffLen=0;c->isOpen=0;return e;
}

const char *NetErrStr(OSErr e)
{
	switch (e) {
		case noErr:					return "OK";
		case openFailed:			return "connection refused";
		case commandTimeout:		return "timed out";
		case connectionDoesntExist:	return "no connection";
		case connectionClosing:		return "connection closing";
		case connectionTerminated:	return "connection dropped";
		case invalidStreamPtr:		return "bad stream";
		case -23000:				return "MacTCP busy";
		default:					return "network error";
	}
}
