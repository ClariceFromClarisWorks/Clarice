/* Small HTTP/1.0 downloader for the trusted development LAN. Not GNU wget.
 * No shell-resident networking. Native HFS services are shared with file tools.
 */
#ifdef WGET_HOST_TEST
#include "../../tests/http/shim.h"
#else
#include "mac-files.h"
#include "network/native.h"
#endif
#include <strings.h>
extern int cli_isatty(int);
#define progress_clock cli_ticks
#define progress_tty() cli_isatty(2)
#define progress_row app_progress
#define progress_columns app_columns
#include "progress.h"
#include "../help-output.h"
#include "../display-size.h"
static NetConn net;
static unsigned char buffer[2048];
static unsigned used,available;
static unsigned long last_data;
static int cancelled,input_error;
static unsigned long download_limit;
static const char *input_failure;
static int poll_input(void){
 if(app_yield()){cancelled=1;return -1;}
 if(used<available)return 1;
 long n=NetAvailable(&net);if(n<0){input_error=n;input_failure="connection closed before response completed";return -1;}
 if(!n){if(cli_ticks()-last_data>1800UL){input_failure="response timed out after 30 seconds without data";return -1;}return 0;}
 if(n>(long)sizeof buffer)n=sizeof buffer;
 n=NetRecv(&net,buffer,(unsigned short)n,1);
 if(n<=0){if(n<0)input_error=n;input_failure="network receive failed";return -1;}used=0;available=(unsigned)n;last_data=cli_ticks();return 1;
}
#include "http-response.inc"
static int number(const char *s,unsigned long *out){
 unsigned long n=0;int digits=0;
 while(*s==' '||*s=='\t')s++;
 while(*s>='0'&&*s<='9'){if(n>214748364UL||(n==214748364UL&&*s>'7'))return 0;n=n*10+*s++-'0';digits=1;}
 while(*s==' '||*s=='\t')s++;if(*s||!digits)return 0;*out=n;return 1;
}
int clarice_main(int argc,char **argv){
 /* Also callable sequentially by the updater within one command invocation. */
 memset(&net,0,sizeof net);used=available=cancelled=input_error=0;last_data=0;input_failure=0;progress_enabled=0;
 const char *url=0,*output=0;char host[260],request[1024],header[1024];
 char http_status[128];
 char native[PATH_CAP],canonical[PATH_CAP],stage[PATH_CAP];
 unsigned short port=80;unsigned long size=0,written=0;short file=-1;
 int owned=0,e=0,result=1,have_length=0,chunked=0;const char *why="invalid arguments";CInfoPBRec pb;
 if(argc==2&&!strcmp(argv[1],"--help"))return help_output(1,
  "Usage:\n"
  "  wget -O NEW_FILE http://HOST[:port]/path\n"
  "Download HTTP to a temporary file on the same volume, then rename it to NEW_FILE. Existing files are refused.\n"
  "HTTP 200 with Content-Length or chunked encoding; headers up to 8 KiB. Idle timeout: 30 seconds.\n"
  "Uses system DNS. No redirects, close-delimited bodies, credentials, TLS or resume.\n"
  "Ctrl+C cancels after pending DNS work. Incomplete files are removed; cleanup errors report the temporary path.\n"
  "Data forks only. Use MacBinary to preserve complete Macintosh files.\n"
  "Example:\n"
  "  wget -O /tmp/index http://192.168.1.1/index.txt\n");
 for(int i=1;i<argc;i++){
  if(!strcmp(argv[i],"-O")&&i+1<argc&&!output)output=argv[++i];
  else if(!url&&argv[i][0]!='-')url=argv[i];else goto done;
 }
 if(!url||!output||strncmp(url,"http://",7))goto done;
 const char *path=strchr(url+7,'/');if(!path||strlen(path)>512)goto done;
 size_t hn=(size_t)(path-url-7);if(!hn||hn>=sizeof host)goto done;memcpy(host,url+7,hn);host[hn]=0;
 char *colon=strchr(host,':');if(colon){unsigned long p;*colon++=0;if(!number(colon,&p)||!p||p>65535)goto done;port=(unsigned short)p;}
 for(const char *p=path;*p;p++)if((unsigned char)*p<=32||(unsigned char)*p>=127||*p=='#')goto done;
 unsigned long ip;
 why="destination resolution";e=checked(output,native,canonical,0);if(e)goto done;
 if(strlen(native)>255){e=bdNamErr;goto done;}
 e=info(native,&pb);if(!e){why="destination exists (not overwritten)";e=dupFNErr;goto done;}if(e!=fnfErr)goto done;e=0;
 why="system hostname resolution";e=NetResolve(host,&ip,app_yield,1800);if(e){if(e==userCanceledErr)cancelled=1;goto done;}
 why="MacTCP connect";e=NetConnect(&net,ip,port);if(e)goto done;
 if(app_yield()){cancelled=1;goto done;}
 snprintf(request,sizeof request,"GET %s HTTP/1.0\r\nHost: %s:%u\r\nConnection: close\r\nAccept-Encoding: identity\r\n\r\n",path,host,port);
 why="HTTP request";e=NetSend(&net,request,strlen(request));if(e)goto done;
 last_data=cli_ticks();
 why="invalid, incomplete or timed-out HTTP headers";
 if(!response_headers(&size,&chunked,http_status,sizeof http_status,&why))goto done;
 have_length=!chunked;
 if(download_limit&&size>download_limit){why="response exceeds requested download limit";goto done;}
 strcpy(stage,native);char *leaf=strrchr(stage,':');if(!leaf){why="invalid native destination";goto done;}
 if((size_t)(leaf-stage)+1+17>255){why="temporary filename exceeds HFS path limit";e=bdNamErr;goto done;}
 why="create temporary file";
 for(unsigned i=0;i<16;i++){snprintf(leaf+1,32,".wget-%08lx-%x",cli_ticks(),i);e=cli_native_create(stage,'BINA','CLRC');if(e!=dupFNErr)break;}
 if(e)goto done;owned=1;e=cli_native_open(stage,0,1,&file);if(e)goto done;
 why="incomplete download";progress_begin();
 struct phr_chunked_decoder decoder;memset(&decoder,0,sizeof decoder);decoder.consume_trailer=1;
 int complete=have_length&&!size;
 while(!complete){int ready=poll_input();if(ready<0)goto done;if(!ready)continue;
  unsigned start=used;size_t decoded=available-used;
  if(chunked){
   ssize_t remaining=phr_decode_chunked(&decoder,(char*)buffer+used,&decoded);
   used=available;
   if(remaining==-1){why="malformed chunked HTTP body";goto done;}
   if(remaining>=0){if(remaining){why="unexpected data after chunked HTTP body";goto done;}complete=1;}
  }else{
   if(decoded>size-written)decoded=size-written;
   used+=(unsigned)decoded;complete=decoded==size-written;
  }
  if(decoded>2147483647UL-written||(download_limit&&(written>download_limit||decoded>download_limit-written))){why="response exceeds requested download limit or HFS capacity";goto done;}
  if(decoded){long n=(long)decoded;
   e=cli_native_write(file,buffer+start,&n);if(e||n!=(long)decoded){if(!e)e=ioErr;why="writing download";goto done;}
   written+=decoded;progress_update(written,size,have_length,0);
  }
 }
 if(app_yield()){cancelled=1;goto done;}
 why="closing download";e=cli_native_close(file);file=-1;if(e)goto done;
 why="committing download";e=cli_native_rename(stage,native);if(e)goto done;owned=0;
 progress_update(written,chunked?written:size,1,1);progress_enabled=0;
 char amount[32];display_size(amount,written);snprintf(header,sizeof header,"Downloaded %s to %s\n",amount,output);say(header);result=0;
done:
 if(progress_enabled){progress_update(written,size,have_length,-1);progress_enabled=0;}
 if(input_failure&&!cancelled)why=input_failure;if(input_error&&!e)e=input_error;
 if(file>=0){int close_error=cli_native_close(file);if(close_error&&!e)e=close_error;}
 int net_error=NetClose(&net);
 if(net_error){snprintf(header,sizeof header,"wget: MacTCP release failed (%d); receive buffer retained for driver safety\n",net_error);errsay(header);if(!result){result=1;why="download saved but network cleanup failed";}}
 if(owned){int remove_error=cli_native_delete(stage);if(remove_error){errsay("wget: cleanup failed; incomplete file retained at native:");errsay(stage);errsay("\n");}}
 if(result){if(cancelled){errsay("wget: cancelled\n");return 130;}errsay("wget: ");errsay(why);if(e){snprintf(header,sizeof header," (Mac error %d)",e);errsay(header);}errsay("\n");}
 return result;
}

/* Shared bounded downloader for the external updater, not a shell service. */
int wget_fetch(const char *url,const char *path,unsigned long maximum){
 char *av[]={"wget","-O",(char*)path,(char*)url};download_limit=maximum;
 int result=clarice_main(4,av);download_limit=0;return result;
}
