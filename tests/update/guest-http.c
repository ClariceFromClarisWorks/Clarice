/* Snow fixture only: serve static HFS files through the production HTTP parser.
 * Does not claim to exercise MacTCP networking. */
#include "../../src/commands/vendor/mactcp/nettcp.h"
#include <Files.h>
#include <string.h>
#include <stdio.h>
static short ref=-1;static long size,left;static unsigned used,hlen;static char header[128];
unsigned long NetParseIP(const char *s){return !strcmp(s,"127.0.0.1")?0x7f000001:0;}
OSErr NetConnect(NetConn *c,unsigned long ip,unsigned short port){(void)c;(void)ip;(void)port;return 0;}
OSErr NetSend(NetConn *c,const void *p,unsigned short n){(void)c;(void)n;char leaf[80],path[256];unsigned char pn[256];if(sscanf(p,"GET /%79s",leaf)!=1)return paramErr;if(strchr(leaf,':')||strchr(leaf,'/')||strstr(leaf,".."))return paramErr;snprintf(path,sizeof path,"Sys6Unix:Update fixture:%s",leaf);pn[0]=strlen(path);memcpy(pn+1,path,pn[0]);int e=HOpen(0,0,pn,fsRdPerm,&ref);if(e){ref=-1;size=left=0;snprintf(header,sizeof header,"HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\n\r\n");}else {e=GetEOF(ref,&size);if(e){FSClose(ref);ref=-1;return e;}left=size;snprintf(header,sizeof header,"HTTP/1.0 200 OK\r\nContent-Length: %ld\r\n\r\n",size);}used=0;hlen=strlen(header);return 0;}
long NetAvailable(NetConn *c){(void)c;return used<hlen||left?2048:-1;}
long NetRecv(NetConn *c,void *p,unsigned short n,unsigned char timeout){(void)c;(void)timeout;unsigned done=0;unsigned char *b=p;while(done<n&&used<hlen)b[done++]=header[used++];if(done<n&&left){long count=n-done;if(count>left)count=left;int e=FSRead(ref,&count,b+done);if(e&&e!=eofErr)return -1;left-=count;done+=count;}return done;}
OSErr NetClose(NetConn *c){(void)c;int e=ref>=0?FSClose(ref):0;ref=-1;return e;}

OSErr NetResolve(const char *s,unsigned long *out,int (*poll)(void),unsigned long timeout){(void)poll;(void)timeout;*out=NetParseIP(s);return *out?0:-50;}
