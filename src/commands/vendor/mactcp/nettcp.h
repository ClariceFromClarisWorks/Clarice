/* Shared MacTCP-compatible client. Foreground calls wait cooperatively using
 * Clarice's existing event loop, retaining every native reference until complete.
 * No networking state or allocation exists before a command uses this transport.
 * NetConn must be zero-initialised; NetClose is required on every exit path.
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
   Yields until connected, cancelled or the five-second open timeout. */
OSErr NetConnect(NetConn *c, unsigned long host, unsigned short port);

/* Non-blocking poll: bytes currently waiting to be read (won't block).
   Returns >=0 unread bytes, connectionClosing for drained orderly EOF, or the
   original negative OSErr on failure. Use this to drive
   a progressive/event-loop receive (read only what's already arrived). */
long NetAvailable(NetConn *c);

/* Receive up to maxlen bytes into buf, waiting up to timeoutSecs for data.
   Returns >=0 bytes received; connectionClosing is orderly EOF. Other negative
   values retain the original OSErr, including timeout/reset. */
long NetRecv(NetConn *c, void *buf, unsigned short maxlen, unsigned char timeoutSecs);

/* Send len bytes (pushed immediately). Yields until acked, cancelled or timeout. */
OSErr NetSend(NetConn *c, const void *buf, unsigned short len);

/* Graceful close + release the stream + free the receive buffer.
   Safe to repeat or call on a half-open / never-opened NetConn. Failed native
   release is retried cooperatively before return, retaining ownership throughout.
   A permanently broken driver can prevent return; freeing its buffer is unsafe.
   Returns the first close/release error even when subsequent cleanup succeeds. */
OSErr NetClose(NetConn *c);

/* Human-readable string for the MacTCP error codes these apps actually hit. */
const char *NetErrStr(OSErr e);

#endif /* NETTCP_H */
