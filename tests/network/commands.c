#define clarice_main ping_main
#include "../../src/commands/ping.c"
#undef clarice_main
#define clarice_main ipconfig_main
#include "../../src/commands/ipconfig.c"
#undef clarice_main
#include <assert.h>
int output_error;
static int failure,calls,diagnostics;
static char output[4096];
void say(const char *s){assert(strlen(output)+strlen(s)<sizeof output);strcat(output,s);}
void errsay(const char *s){(void)s;diagnostics++;}
void error(const char *s,int e){(void)s;assert(e);diagnostics++;}
int app_yield(void){return 0;}
OSErr NetAddress(unsigned long *a,unsigned long *m){*a=0x0a000002;*m=0xffffff00;return failure;}
OSErr NetResolve(const char *n,unsigned long *a,NetPoll p,unsigned long t){(void)n;(void)p;(void)t;*a=0x0a000001;return failure==2?-23045:0;}
OSErr NetEcho(unsigned long a,unsigned short s,NetPoll p,unsigned long *t){assert(a==0x0a000001&&s==1);(void)p;calls++;*t=3;return failure;}
void NetFormatIP(unsigned long a,char s[16]){snprintf(s,16,"%lu.%lu.%lu.%lu",a>>24,(a>>16)&255,(a>>8)&255,a&255);}
int main(void){
 char *ip[]={"ipconfig"},*ph[]={"ping","--help"},*ih[]={"ipconfig","--help"};
 assert(!ipconfig_main(1,ip)&&strstr(output,"10.0.0.2")&&strstr(output,"255.255.255.0"));
 output[0]=0;assert(!ping_main(2,ph)&&strstr(output,"usage:"));output[0]=0;assert(!ipconfig_main(2,ih));
 char *p[]={"ping","-c","2","-W","1","example.test"};output[0]=0;
 assert(!ping_main(6,p)&&calls==2&&strstr(output,"2 transmitted, 2 received"));
 failure=-23035;calls=0;assert(ping_main(6,p)==1&&calls==2);
 failure=-128;assert(ping_main(6,p)==130);
 failure=2;assert(ping_main(6,p)==1);failure=0;
 const char *bad[]={"0","-1","65536","99999999999999999999999","x"};
 for(unsigned i=0;i<sizeof bad/sizeof *bad;i++){p[2]=(char*)bad[i];assert(ping_main(6,p)==2);}
 p[2]="2";p[4]="61";assert(ping_main(6,p)==2);p[4]="1";
 output_error=1;assert(ping_main(6,p)==1&&ipconfig_main(1,ip)==1);output_error=0;
 failure=-43;assert(ipconfig_main(1,ip)==1);ph[1]="--unknown";assert(ipconfig_main(2,ph)==2);
 puts("PASS network command options, status, output errors, numeric formatting, timeout and cancellation statuses");
}
