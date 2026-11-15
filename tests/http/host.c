#define WGET_HOST_TEST
#include "../../src/commands/wget.c"
#include <assert.h>
static unsigned char response[16384],saved[16384];
static unsigned length,pos,bytes,ticks,calls;
static int exists,temporary,committed,network,stop,stall,write_error,close_error,rename_error,create_error,cleanup_error;
static char diagnostics[4096];
int cli_isatty(int fd){(void)fd;return 0;}
void app_progress(const char *s,int done){(void)s;(void)done;assert(0);}
int app_yield(void){return stop&&++calls>5;}
unsigned long cli_ticks(void){return ++ticks;}
int checked(const char *a,char *b,char *c,int d){(void)d;strcpy(b,"Disk:");strcat(b,a);strcpy(c,a);return 0;}
int info(const char *a,CInfoPBRec *b){(void)a;(void)b;return exists?0:fnfErr;}
unsigned long NetParseIP(const char *a){return !strcmp(a,"127.0.0.1")?0x7f000001:0;}
int NetConnect(NetConn *a,unsigned long b,unsigned short c){(void)a;(void)b;(void)c;network++;return 0;}
int NetSend(NetConn *a,const void *b,unsigned short n){(void)a;assert(n&&strstr(b,"GET /test HTTP/1.0\r\n"));return 0;}
long NetAvailable(NetConn *a){(void)a;return stall?0:pos<length?(long)(length-pos):-1;}
long NetRecv(NetConn *a,void *b,unsigned short n,unsigned char c){(void)a;(void)c;if(n>17)n=17;memcpy(b,response+pos,n);pos+=n;return n;}
int NetClose(NetConn *a){(void)a;return 0;}
void say(const char *s){(void)s;}
void errsay(const char *s){if(strlen(diagnostics)+strlen(s)<sizeof diagnostics)strcat(diagnostics,s);}
int cli_native_create(const char *a,unsigned long b,unsigned long c){(void)a;(void)b;(void)c;if(create_error)return -44;assert(!temporary);temporary=1;return 0;}
int cli_native_open(const char *a,int b,int c,short *d){(void)a;(void)b;(void)c;*d=1;return 0;}
int cli_native_write(short a,const void *b,long *n){(void)a;if(write_error)return ioErr;assert(*n>=0&&(unsigned long)bytes+(unsigned long)*n<sizeof saved);memcpy(saved+bytes,b,*n);bytes+=*n;return 0;}
int cli_native_close(short a){(void)a;return close_error?ioErr:0;}
int cli_native_rename(const char *a,const char *b){(void)a;(void)b;if(rename_error)return ioErr;assert(!exists);committed=1;temporary=0;return 0;}
int cli_native_delete(const char *a){(void)a;if(cleanup_error)return ioErr;temporary=0;return 0;}
static void reset(const char *header){memset(&net,0,sizeof net);used=available=cancelled=0;last_data=0;input_failure=0;progress_enabled=0;length=strlen(header);memcpy(response,header,length);pos=bytes=ticks=calls=0;exists=temporary=committed=network=stop=stall=write_error=close_error=rename_error=create_error=cleanup_error=0;diagnostics[0]=0;}
static int run(void){char *av[]={"wget","-O","result","http://127.0.0.1/test"};return clarice_main(4,av);}
int main(void){
 reset("HTTP/1.0 200 OK\r\nContent-Length: 4096\r\n\r\n");for(unsigned i=0;i<4096;i++)response[length++]=i;
 assert(run()==0&&committed&&bytes==4096&&!temporary);for(unsigned i=0;i<4096;i++)assert(saved[i]==(unsigned char)i);
 const char *bad[]={"HTTP/1.0 404 Missing\r\nContent-Length: 0\r\n\r\n","HTTP/1.0 200 OK\r\n\r\n","HTTP/1.0 200 OK\r\nContent-Length: 5\r\n\r\nabc","HTTP/1.0 200 OK\r\nContent-Length: 0\r\nContent-Length: 0\r\n\r\n","HTTP/1.0 200 OK\r\nContent-Length: 2147483648\r\n\r\n","HTTP/1.0 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n","HTTP/1.0 200 OK\n\n"};
 for(unsigned i=0;i<sizeof bad/sizeof *bad;i++){reset(bad[i]);assert(run()!=0&&!committed&&!temporary);}
 for(int mode=0;mode<7;mode++){reset("HTTP/1.0 200 OK\r\nContent-Length: 3\r\n\r\nabc");switch(mode){case 0:exists=1;break;case 1:write_error=1;break;case 2:close_error=1;break;case 3:rename_error=1;break;case 4:create_error=1;break;case 5:stop=1;break;case 6:stall=1;break;}int r=run();assert(r!=0&&!committed&&!temporary);if(mode==0)assert(!network&&exists);if(mode==5)assert(r==130);}
 reset("HTTP/1.0 200 OK\r\nContent-Length: 3\r\n\r\na");cleanup_error=1;assert(run()!=0&&temporary&&strstr(diagnostics,"cleanup failed"));
 reset("HTTP/1.0 200 OK\r\nContent-Length: 0\r\n\r\n");assert(run()==0&&committed&&!bytes);
 reset("HTTP/1.1 503 Service Unavailable\r\nContent-Length: 0\r\n\r\n");assert(run()!=0&&strstr(diagnostics,"503 Service Unavailable")&&!committed);
 reset("HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\n\r\n");assert(run()!=0&&strstr(diagnostics,"404 Not Found")&&!committed);
 reset("HTTP/1.0 200 OK\r\nContent-Length: 5\r\n\r\na");assert(run()!=0&&strstr(diagnostics,"connection closed")&&!committed);
 reset("HTTP/1.0 200 OK\r\nContent-Length: 3\r\n\r\nabc");assert(wget_fetch("http://127.0.0.1/test","result",2)!=0&&!temporary&&!committed&&strstr(diagnostics,"download limit"));
 reset("HTTP/1.0 200 OK\r\nContent-Length: 3\r\n\r\nabc");assert(wget_fetch("http://127.0.0.1/test","result",3)==0&&committed&&bytes==3);
 puts("PASS: binary, fragmented headers/body, zero length, truncation, status, length bounds, duplicates, encoding, framing, existing file, write/close/rename/create failures, cancel, timeout, cleanup recovery path");return 0;
}
