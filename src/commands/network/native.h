/* Invocation-local services over the installed Macintosh stack. No sockets. */
#ifndef CLARICE_NATIVE_NETWORK_H
#define CLARICE_NATIVE_NETWORK_H
#include "../vendor/mactcp/nettcp.h"
typedef int (*NetPoll)(void);
OSErr NetAddress(unsigned long *address,unsigned long *mask);
/* Cancellation/deadline stop useful work, then drain native callbacks before
   unloading. Apple DNR has no documented per-query cancellation operation. */
OSErr NetResolve(const char *name,unsigned long *address,NetPoll poll,unsigned long ticks);
OSErr NetEcho(unsigned long address,unsigned short seconds,NetPoll poll,unsigned long *ticks);
void NetFormatIP(unsigned long address,char text[16]);
#endif
