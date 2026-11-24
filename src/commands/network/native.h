/* Invocation-local services over the installed Macintosh stack. No sockets. */
#ifndef CLARICE_NATIVE_NETWORK_H
#define CLARICE_NATIVE_NETWORK_H
#include "../vendor/mactcp/nettcp.h"
typedef int (*NetPoll)(void);
OSErr NetAddress(unsigned long *address,unsigned long *mask);
/* Cancellation/deadline stop useful work, then drain native callbacks before
   unloading. Apple DNR has no documented per-query cancellation operation. */
OSErr NetResolve(const char *name,unsigned long *address,NetPoll poll,unsigned long ticks);
/* Adapter error, not a native Mac OS error: callback claimed success but
   its echoed bytes or timestamps did not describe our request. */
#define NET_ECHO_INVALID (-32700)
OSErr NetEcho(unsigned long address,unsigned short seconds,NetPoll poll,unsigned long *ticks);
void NetFormatIP(unsigned long address,char text[16]);
#endif
