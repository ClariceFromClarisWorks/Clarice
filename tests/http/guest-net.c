/* Test-only HTTP transport: production parser and native HFS remain unchanged. */
#include "../../src/commands/vendor/mactcp/nettcp.h"
#include <string.h>
#include <stdio.h>
extern unsigned long cli_ticks(void);
static char header[128];static unsigned long offset,limit,next;static unsigned hlen;static int fault;
unsigned long NetParseIP(const char *s){(void)s;return 0x7f000001;}
OSErr NetConnect(NetConn *n,unsigned long ip,unsigned short port){(void)n;(void)ip;(void)port;return 0;}
OSErr NetSend(NetConn *n,const void *p,unsigned short len){(void)n;(void)len;fault=strstr(p,"/404 ")?404:strstr(p,"/503 ")?503:strstr(p,"/truncated ")?1:0;limit=fault>=400?0:16384;snprintf(header,sizeof header,"HTTP/1.0 %s\r\nContent-Length: %lu\r\n\r\n",fault==404?"404 Not Found":fault==503?"503 Service Unavailable":"200 OK",limit);hlen=strlen(header);offset=0;next=cli_ticks();return 0;}
long NetAvailable(NetConn *n){(void)n;if(offset>=hlen+limit||(fault==1&&offset>=hlen+512))return -1;return cli_ticks()<next?0:128;}
long NetRecv(NetConn *n,void *p,unsigned short count,unsigned char timeout){(void)n;(void)timeout;unsigned char *b=p;unsigned i=0;for(;i<count&&offset<hlen+limit;i++,offset++)b[i]=offset<hlen?(unsigned char)header[offset]:(unsigned char)(offset-hlen);next=cli_ticks()+2;return i;}
OSErr NetClose(NetConn *n){(void)n;return 0;}

OSErr NetResolve(const char *s,unsigned long *out,int (*poll)(void),unsigned long timeout){(void)poll;(void)timeout;*out=NetParseIP(s);return *out?0:-50;}
