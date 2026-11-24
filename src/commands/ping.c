#include "network/command.h"
static int positive(const char *s,unsigned max,unsigned *out){unsigned n=0;if(!*s)return 0;while(*s){if(*s<'0'||*s>'9'||n>max/10)return 0;n=n*10+*s++-'0';if(n>max)return 0;}if(!n)return 0;*out=n;return 1;}
/* Rounded percentage without overflowing long-running counters or pulling in
   64-bit division support on the 68000. Runs only once at command completion. */
static unsigned loss_percent(unsigned sent,unsigned received){
 if(!sent)return 0;
 unsigned remainder=sent/2,loss=0;
 for(unsigned i=0;i<100;i++){
  if(remainder>=received){remainder-=received;++loss;}
  else remainder+=sent-received;
 }
 return loss;
}
int clarice_main(int argc,char **argv){
 unsigned count=0,timeout=2;const char *host=0;
 if(argc==2&&!strcmp(argv[1],"--help")){say("usage: ping [-c COUNT] [-W SECONDS] HOST\nNative Macintosh ICMP echo; IPv4 or system DNR hostname.\nDefaults: continuous until Ctrl+C, 2-second reply timeout. Count <=65535; timeout <=60.\nRTT in milliseconds, rounded from native ticks (~1/60s); <17 ms means\nless than one measured tick, not zero network delay. One-second probe interval. Ctrl+C cancels\nafter outstanding native callbacks drain; DNS cancellation can take longer.\nNo raw sockets, TTL display or network reconfiguration.\nStatus: 0 all replies; 1 loss/error; 2 usage; 130 cancelled.\n");return 0;}
 for(int i=1;i<argc;i++){if(!strcmp(argv[i],"-c")&&i+1<argc){if(!positive(argv[++i],65535,&count))return 2;}else if(!strcmp(argv[i],"-W")&&i+1<argc){if(!positive(argv[++i],60,&timeout))return 2;}else if(!host&&argv[i][0]!='-')host=argv[i];else return 2;}
 if(!host){errsay("usage: ping [-c COUNT] [-W SECONDS] HOST\n");return 2;}
 unsigned long address;int e=NetResolve(host,&address,app_yield,1800);if(e){error("ping: resolve",e);return e==userCanceledErr?130:1;}
 char ip[16],line[160];NetFormatIP(address,ip);unsigned received=0,sent=0;
 int cancelled=0;
 for(;!count||sent<count;){unsigned long ticks=0,start=TickCount();e=NetEcho(address,timeout,app_yield,&ticks);++sent;
  if(e==userCanceledErr){cancelled=1;break;}
  if(!e){++received;
   if(ticks)snprintf(line,sizeof line,"reply from %s: seq=%u time=%lu ms\n",ip,sent,(ticks*1000UL+30)/60);
   else snprintf(line,sizeof line,"reply from %s: seq=%u time<17 ms\n",ip,sent);
   say(line);
  }else if(e==(-23035)){snprintf(line,sizeof line,"request timeout: seq=%u\n",sent);say(line);}
  else if(e==NET_ECHO_INVALID)errsay("ping: invalid echo reply data or timestamps\n");
  else error("ping: echo",e);
  if(output_error)return 1;
  if(e&&e!=(-23035)&&e!=NET_ECHO_INVALID)break;
  while((!count||sent<count)&&(unsigned long)(TickCount()-start)<60){if(app_yield()){cancelled=1;break;}SystemTask();}
  if(cancelled)break;
 }
 unsigned loss=loss_percent(sent,received);
 snprintf(line,sizeof line,"%u transmitted, %u received, %u%% packet loss\n",sent,received,loss);say(line);
 return cancelled?130:output_error||received!=sent;
}
