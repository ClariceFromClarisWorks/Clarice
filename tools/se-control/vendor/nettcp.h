/*
 * nettcp.h — minimal synchronous MacTCP client for the Macintosh Plus.
 *
 * This is the shared TCP transport for the WiFi-era Plus apps. It replaces the
 * RetroWiFi-SI serial-modem path: instead of dialing a Hayes modem and talking
 * to the mini multiplexer over RS-422, we open a real TCP socket through the
 * BlueSCSI DaynaPORT (Ethernet emulation) + MacTCP.
 *
 * Reads/sends are synchronous and bounded. Connection attempts use native
 * asynchronous I/O with an event-poll callback, so reconnects do not freeze
 * Finder while the remote host is unavailable. Numeric local addresses only.
 */
#ifndef NETTCP_H
#define NETTCP_H

#include <MacTypes.h>

typedef struct NetConn {
	Ptr	stream;		/* MacTCP StreamPtr (0 = none)            */
	Ptr				rcvBuff;	/* MacTCP-owned receive ring (we allocate) */
	unsigned long	rcvBuffLen;
	short			isOpen;		/* 1 once ActiveOpen succeeds              */
} NetConn;

/* Open the MacTCP driver (.IPP). Idempotent — safe to call repeatedly.
   Returns noErr, or an OSErr if MacTCP/DaynaPORT isn't installed. */
OSErr NetInit(void);

/* Build an ip_addr (host byte order, as MacTCP wants) from "a.b.c.d".
   Returns 0 on a malformed string. */
unsigned long NetParseIP(const char *dotted);

/* Create a stream and actively open a connection to host:port.
   Uses an asynchronous open, yielding through poll until completion or timeout. */
typedef int (*NetPoll)(void); /* service events; nonzero requests cancellation */
OSErr NetConnect(NetConn *c, unsigned long host, unsigned short port, NetPoll poll);

/* Non-blocking poll: bytes currently waiting to be read (won't block).
   Returns >=0 unread bytes, or -1 if the connection is gone. Use this to drive
   a progressive/event-loop receive (read only what's already arrived). */
long NetAvailable(NetConn *c);

/* Receive up to maxlen bytes into buf, waiting up to timeoutSecs for data.
   Returns: >0 = bytes received; 0 = peer closed OR timed out (no more data);
            <0 = (-OSErr) on a real error. Loop until <=0 for a bulk read. */
long NetRecv(NetConn *c, void *buf, unsigned short maxlen, unsigned char timeoutSecs);

/* Send len bytes (pushed immediately). Blocks until acked or timeout. */
OSErr NetSend(NetConn *c, const void *buf, unsigned short len);

/* Abort ONLY this stream, release it, then free its receive buffer.
   On release failure retain ownership and return the error; never reuse or free
   the driver-owned storage. Safe on half-open / never-opened connections. */
OSErr NetClose(NetConn *c);

/* Human-readable string for the MacTCP error codes these apps actually hit. */
const char *NetErrStr(OSErr e);

#endif /* NETTCP_H */
