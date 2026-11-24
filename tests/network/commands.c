#include <assert.h>
#include "../../src/help-output.h"
static char output[4096];
static long capture_help(int fd,const void *data,unsigned long count){
 assert(fd==1&&strlen(output)+count<sizeof output);
 strncat(output,data,count);
 return count;
}
static int network_help(int fd,const char *text){return help_format(66,fd,text,capture_help,0);}
#define help_output network_help
#define clarice_main ping_main
#include "../../src/commands/ping.c"
#undef clarice_main
#define clarice_main ipconfig_main
#include "../../src/commands/ipconfig.c"
#undef clarice_main
#undef help_output
int output_error;
static int failure,calls,diagnostics,cancel,cancel_after;
static unsigned long now,reply_ticks=3;
void say(const char *s){assert(strlen(output)+strlen(s)<sizeof output);strcat(output,s);}
void errsay(const char *s){(void)s;diagnostics++;}
void error(const char *s,int e){(void)s;assert(e);diagnostics++;}
int app_yield(void){return cancel||(cancel_after&&calls>=cancel_after);}
unsigned long TickCount(void){return now;}
void SystemTask(void){now++;}
OSErr NetAddress(unsigned long *a,unsigned long *m){*a=0x0a000002;*m=0xffffff00;return failure;}
OSErr NetResolve(const char *n,unsigned long *a,NetPoll p,unsigned long t){(void)n;(void)p;(void)t;*a=0x0a000001;return failure==2?-23045:0;}
OSErr NetEcho(unsigned long a,unsigned short s,NetPoll p,unsigned long *t){assert(a==0x0a000001&&s==1);(void)p;if(calls)assert(now>=60UL*(unsigned)calls);calls++;*t=reply_ticks;return failure;}
void NetFormatIP(unsigned long a,char s[16]){snprintf(s,16,"%lu.%lu.%lu.%lu",a>>24,(a>>16)&255,(a>>8)&255,a&255);}
static int run_ping(int argc,char **argv){calls=0;now=0;return ping_main(argc,argv);}
int main(void){
 assert(loss_percent(0,0)==0);
 for(unsigned n=1;n<1000;n++)for(unsigned r=0;r<=n;r++)
  assert(loss_percent(n,r)==((n-r)*100+n/2)/n);
 assert(loss_percent(4294967295U,2147483647U)==50);
 assert(loss_percent(4294967295U,4294967295U)==0);
 assert(loss_percent(4294967295U,0)==100);
 char *ip[]={"ipconfig"},*ph[]={"ping","--help"},*ih[]={"ipconfig","--help"};
 assert(!ipconfig_main(1,ip)&&strstr(output,"10.0.0.2")&&strstr(output,"255.255.255.0"));
 output[0]=0;assert(!run_ping(2,ph)&&strstr(output,"Usage:\n  ping"));
 output[0]=0;assert(!ipconfig_main(2,ih)&&strstr(output,"Usage:\n  ipconfig"));
 char *p[]={"ping","-c","2","-W","1","example.test"};output[0]=0;
 assert(!run_ping(6,p)&&calls==2&&strstr(output,"2 transmitted, 2 received"));
 assert(strstr(output,"time=50 ms")&&strstr(output,"0% packet loss"));
 reply_ticks=0;calls=0;output[0]=0;assert(!run_ping(6,p)&&strstr(output,"time<17 ms"));
 reply_ticks=1;calls=0;output[0]=0;assert(!run_ping(6,p)&&strstr(output,"time=17 ms"));
 reply_ticks=2;calls=0;output[0]=0;assert(!run_ping(6,p)&&strstr(output,"time=33 ms"));
 char *continuous[]={"ping","-W","1","example.test"};
 cancel_after=7;output[0]=0;assert(run_ping(4,continuous)==130&&calls==7);cancel_after=0;
 cancel=1;assert(run_ping(6,p)==130);cancel=0;
 failure=NET_ECHO_INVALID;assert(run_ping(6,p)==1);failure=0;
 failure=-23035;calls=0;output[0]=0;assert(run_ping(6,p)==1&&calls==2);
 assert(strstr(output,"100% packet loss")&&!strstr(output,"reply from"));
 failure=-128;assert(run_ping(6,p)==130);
 failure=2;assert(run_ping(6,p)==1);failure=0;
 const char *bad[]={"0","-1","65536","99999999999999999999999","x"};
 for(unsigned i=0;i<sizeof bad/sizeof *bad;i++){p[2]=(char*)bad[i];assert(run_ping(6,p)==2);}
 p[2]="2";p[4]="61";assert(run_ping(6,p)==2);p[4]="1";
 output_error=1;assert(run_ping(6,p)==1&&ipconfig_main(1,ip)==1);output_error=0;
 failure=-43;assert(ipconfig_main(1,ip)==1);ph[1]="--unknown";assert(ipconfig_main(2,ph)==2);
 puts("PASS network command options, status, output errors, numeric formatting, timeout and cancellation statuses");
}
