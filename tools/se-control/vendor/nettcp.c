/*
 * nettcp.c — minimal synchronous MacTCP client. See nettcp.h.
 */
#include <Devices.h>
#include <Memory.h>
#include <OSUtils.h>
#include <Multiverse.h> /* pinned Multiversal includes MacTCP declarations */
#include "nettcp.h"
#include <string.h>
/* Local proof hardening: zero parameter blocks and bounded five-second calls. */

#define IPP_RCVBUF_LEN	16384UL		/* MacTCP wants >= 4*MTU; 16K is comfortable */

static short	gIPPRefNum = 0;
static Boolean	gDriverOpen = false;

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
			if (part > 255) return 0;
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

OSErr NetConnect(NetConn *c, unsigned long host, unsigned short port, NetPoll poll)
{
	TCPiopb pb;
	memset(&pb, 0, sizeof pb);
	OSErr err;

	/* A failed release still owns its ring. Do not allocate over that state. */
	err = NetClose(c);
	if (err != noErr) return err;
	if (poll && poll()) return -128;

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
	pb.csParam.open.commandTimeoutValue = 0;
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
	/* Native asynchronous open keeps Finder responsive during reconnects. No
	 * completion callback; the PB stays alive until the driver completes it.
	 * Apple PLAT04: KillIO cannot cancel MacTCP; abort this stream instead. */
	unsigned long started = TickCount();
	int stopped = 0;
	err = PBControlAsync((ParmBlkPtr)&pb);
	if (err >= noErr) {
		while (*(volatile short *)&pb.ioResult > 0) {
			int cancel = poll && poll();
			if (!stopped && (cancel || TickCount()-started >= 60UL*6)) {
				TCPiopb abortPB;
				memset(&abortPB, 0, sizeof abortPB);
				abortPB.ioCRefNum = gIPPRefNum;
				abortPB.csCode = TCPAbort;
				abortPB.tcpStream = c->stream;
				(void)PBControlSync((ParmBlkPtr)&abortPB);
				stopped = cancel ? -128 : commandTimeout;
			}
			/* Even after cancellation, never return a pending stack PB to caller.
			 * A wedged driver cannot safely be unloaded by this application. */
			SystemTask();
		}
		err = stopped ? stopped : pb.ioResult;
	}
	if (err != noErr) {
		(void)NetClose(c); /* preserve failed release for a later retry */
		return err;
	}

	c->isOpen = 1;
	return noErr;
}

long NetAvailable(NetConn *c)
{
	TCPiopb pb;
	memset(&pb, 0, sizeof pb);
	OSErr err;

	if (!c->isOpen)
		return -1;

	pb.ioCRefNum = gIPPRefNum;
	pb.csCode = TCPStatus;
	pb.tcpStream = c->stream;
	pb.csParam.status.userDataPtr = nil;
	err = PBControlSync((ParmBlkPtr)&pb);
	if (err != noErr)
		return -1;					/* connection gone */
	/* Drain queued data, then recognize peer FIN/closed states. MacTCP
	 * Programmer's Guide TCPStatus: ESTABLISHED = 8. This active-only
	 * client has already completed TCPActiveOpen before polling here. */
	if (!pb.csParam.status.amtUnreadData && pb.csParam.status.connectionState != 8)
		return -1;
	return (long)pb.csParam.status.amtUnreadData;
}

long NetRecv(NetConn *c, void *buf, unsigned short maxlen, unsigned char timeoutSecs)
{
	TCPiopb pb;
	memset(&pb, 0, sizeof pb);
	OSErr err;

	if (!c->isOpen)
		return -1;

	pb.ioCRefNum = gIPPRefNum;
	pb.csCode = TCPRcv;
	pb.tcpStream = c->stream;
	pb.csParam.receive.commandTimeoutValue = (SInt8)timeoutSecs;
	pb.csParam.receive.rcvBuff = (Ptr)buf;
	pb.csParam.receive.rcvBuffLen = maxlen;
	pb.csParam.receive.userDataPtr = nil;
	err = PBControlSync((ParmBlkPtr)&pb);

	if (err == noErr)
		return (long)pb.csParam.receive.rcvBuffLen;	/* bytes actually received */

	/* peer closed, or nothing more arrived in time → "no more data" */
	if (err == connectionClosing || err == connectionTerminated ||
		err == commandTimeout || err == connectionDoesntExist)
		return 0;

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
	return PBControlSync((ParmBlkPtr)&pb);
}

OSErr NetClose(NetConn *c)
{
	TCPiopb pb;
	memset(&pb, 0, sizeof pb);

	if (c->stream != 0) {
		/* This helper has no unsent file data. Abort avoids a graceful-close
		 * network wait and affects no other application's connections. */
		pb.ioCRefNum = gIPPRefNum;
		pb.csCode = TCPAbort;
		pb.tcpStream = c->stream;
		(void)PBControlSync((ParmBlkPtr)&pb);

		pb.ioCRefNum = gIPPRefNum;
		pb.csCode = TCPRelease;
		pb.tcpStream = c->stream;
		OSErr err = PBControlSync((ParmBlkPtr)&pb);
		if (err != noErr) { c->isOpen = 0; return err; }
		c->stream = 0;
	}
	if (c->rcvBuff != nil) {
		DisposePtr(c->rcvBuff);
		c->rcvBuff = nil;
	}
	c->isOpen = 0;
	c->rcvBuffLen = 0;
	return noErr;
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
