#include "network/command.h"
static int positive(const char *s,unsigned max,unsigned *out){unsigned n=0;if(!*s)return 0;while(*s){if(*s<'0'||*s>'9'||n>max/10)return 0;n=n*10+*s++-'0';if(n>max)return 0;}if(!n)return 0;*out=n;return 1;}
int clarice_main(int argc,char **argv){
 unsigned count=4,timeout=2;const char *host=0;
 if(argc==2&&!strcmp(argv[1],"--help")){say("usage: ping [-c COUNT] [-W SECONDS] HOST\nNative Macintosh ICMP echo; IPv4 or system DNR hostname.\nDefaults: 4 probes, 2-second reply timeout. Count <=65535; timeout <=60.\nReports guest ticks (about 1/60s), not sub-tick precision. Ctrl+C cancels\nafter outstanding native callbacks drain; DNS cancellation can take longer.\nNo raw sockets, TTL display or network reconfiguration.\nStatus: 0 all replies; 1 loss/error; 2 usage; 130 cancelled.\n");return 0;}
 for(int i=1;i<argc;i++){if(!strcmp(argv[i],"-c")&&i+1<argc){if(!positive(argv[++i],65535,&count))return 2;}else if(!strcmp(argv[i],"-W")&&i+1<argc){if(!positive(argv[++i],60,&timeout))return 2;}else if(!host&&argv[i][0]!='-')host=argv[i];else return 2;}
 if(!host){errsay("usage: ping [-c COUNT] [-W SECONDS] HOST\n");return 2;}
 unsigned long address;int e=NetResolve(host,&address,app_yield,1800);if(e){error("ping: resolve",e);return e==userCanceledErr?130:1;}
 char ip[16],line[160];NetFormatIP(address,ip);unsigned received=0,sent=0;
 for(;sent<count;){unsigned long ticks=0;e=NetEcho(address,timeout,app_yield,&ticks);++sent;
  if(e==userCanceledErr)return 130;
  if(!e){++received;snprintf(line,sizeof line,"reply from %s: seq=%u time=%lu ticks\n",ip,sent,ticks);say(line);}else error("ping: echo",e);
  if(output_error)return 1;
  if(e&&e!=(-23035))break;
 }
 snprintf(line,sizeof line,"%u transmitted, %u received\n",sent,received);say(line);return output_error||received!=sent;
}
