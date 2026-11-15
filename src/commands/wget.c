/* Small HTTP/1.0 downloader for the trusted development LAN. Not GNU wget.
 * No shell-resident networking. Native HFS services are shared with file tools.
 */
#ifdef WGET_HOST_TEST
#include "../../tests/http/shim.h"
#else
#include "mac-files.h"
#include "vendor/mactcp/nettcp.h"
#endif
#include <strings.h>
extern int cli_isatty(int);
#define progress_clock cli_ticks
#define progress_tty() cli_isatty(2)
#define progress_row app_progress
#define progress_columns app_columns
#include "progress.h"
#include "../display-size.h"
static NetConn net;
static unsigned char buffer[2048];
static unsigned used,available;
static unsigned long last_data;
static int cancelled;
static unsigned long download_limit;
static const char *input_failure;
static int poll_input(void){
 if(app_yield()){cancelled=1;return -1;}
 if(used<available)return 1;
 long n=NetAvailable(&net);if(n<0){input_failure="connection closed before response completed";return -1;}
 if(!n){if(cli_ticks()-last_data>1800UL){input_failure="response timed out after 30 seconds without data";return -1;}return 0;}
 if(n>(long)sizeof buffer)n=sizeof buffer;
 n=NetRecv(&net,buffer,(unsigned short)n,1);
 if(n<=0){input_failure="network receive failed";return -1;}used=0;available=(unsigned)n;last_data=cli_ticks();return 1;
}
static int line(char *out,unsigned cap,unsigned long *budget){
 unsigned n=0;int cr=0;
 for(;;){int ready=poll_input();if(ready<0)return 0;if(!ready)continue;
  unsigned char c=buffer[used++];if(!*budget)return 0;--*budget;
  if(c=='\n'){if(!cr)return 0;out[n]=0;return 1;}
  if(cr)return 0;if(c=='\r'){cr=1;continue;}
  if(c<32&&c!='\t')return 0;if(c==127||n+1>=cap)return 0;out[n++]=c;
 }
}
static int number(const char *s,unsigned long *out){
 unsigned long n=0;int digits=0;
 while(*s==' '||*s=='\t')s++;
 while(*s>='0'&&*s<='9'){if(n>214748364UL||(n==214748364UL&&*s>'7'))return 0;n=n*10+*s++-'0';digits=1;}
 while(*s==' '||*s=='\t')s++;if(*s||!digits)return 0;*out=n;return 1;
}
int clarice_main(int argc,char **argv){
 /* Also callable sequentially by the updater within one command invocation. */
 memset(&net,0,sizeof net);used=available=cancelled=0;last_data=0;input_failure=0;progress_enabled=0;
 const char *url=0,*output=0;char host[32],request[768],header[1024];
 char http_status[128];
 char native[PATH_CAP],canonical[PATH_CAP],stage[PATH_CAP];
 unsigned short port=80;unsigned long size=0,written=0;short file=-1;
 int owned=0,e=0,result=1,have_length=0;const char *why="invalid arguments";CInfoPBRec pb;
 if(argc==2&&!strcmp(argv[1],"--help")){
  say("usage: wget -O NEW_FILE http://IPv4[:port]/path\nStreams HTTP GET to a same-volume temporary file, closes, then renames.\nExisting destinations are refused. Incomplete files are removed; cleanup\nfailures report the temporary path. Ctrl+C cancels. 30-second input timeout.\nHTTP 200 with Content-Length required; no redirects, chunked encoding,\nDNS, credentials, TLS or resume. Data fork only; use MacBinary for Mac apps.\nThis is Clarice's small downloader, not GNU wget.\n");return 0;
 }
 for(int i=1;i<argc;i++){
  if(!strcmp(argv[i],"-O")&&i+1<argc&&!output)output=argv[++i];
  else if(!url&&argv[i][0]!='-')url=argv[i];else goto done;
 }
 if(!url||!output||strncmp(url,"http://",7))goto done;
 const char *path=strchr(url+7,'/');if(!path||strlen(path)>512)goto done;
 size_t hn=(size_t)(path-url-7);if(!hn||hn>=sizeof host)goto done;memcpy(host,url+7,hn);host[hn]=0;
 char *colon=strchr(host,':');if(colon){unsigned long p;*colon++=0;if(!number(colon,&p)||!p||p>65535)goto done;port=(unsigned short)p;}
 for(const char *p=path;*p;p++)if((unsigned char)*p<=32||(unsigned char)*p>=127||*p=='#')goto done;
 unsigned long ip=NetParseIP(host);if(!ip)goto done;
 why="destination resolution";e=checked(output,native,canonical,0);if(e)goto done;
 if(strlen(native)>255){e=bdNamErr;goto done;}
 e=info(native,&pb);if(!e){why="destination exists (not overwritten)";e=dupFNErr;goto done;}if(e!=fnfErr)goto done;e=0;
 why="MacTCP connect";e=NetConnect(&net,ip,port);if(e)goto done;
 if(app_yield()){cancelled=1;goto done;}
 snprintf(request,sizeof request,"GET %s HTTP/1.0\r\nHost: %s:%u\r\nConnection: close\r\nAccept-Encoding: identity\r\n\r\n",path,host,port);
 why="HTTP request";e=NetSend(&net,request,strlen(request));if(e)goto done;
 last_data=cli_ticks();unsigned long budget=8192;
 why="invalid, incomplete or timed-out HTTP headers";
 if(!line(header,sizeof header,&budget))goto done;
 if((strncmp(header,"HTTP/1.0 ",9)&&strncmp(header,"HTTP/1.1 ",9))||strlen(header)<12||header[9]<'1'||header[9]>'5'||header[10]<'0'||header[10]>'9'||header[11]<'0'||header[11]>'9'||(header[12]&&header[12]!=' ')){why="malformed HTTP status line";goto done;}
 snprintf(http_status,sizeof http_status,"HTTP %.100s",header+9);
 if(strncmp(header+9,"200",3)){why=http_status;goto done;}
 for(;;){if(!line(header,sizeof header,&budget))goto done;if(!*header)break;
  char *v=strchr(header,':');if(!v)goto done;*v++=0;
  if(!strcasecmp(header,"Content-Length")){if(have_length||!number(v,&size))goto done;have_length=1;}
  if(!strcasecmp(header,"Transfer-Encoding")){why="transfer encoding unsupported";goto done;}
  if(!strcasecmp(header,"Content-Encoding")){while(*v==' '||*v=='\t')v++;if(strcasecmp(v,"identity")){why="content encoding unsupported";goto done;}}
 }
 if(!have_length){why="Content-Length required";goto done;}
 if(download_limit&&size>download_limit){why="response exceeds requested download limit";goto done;}
 strcpy(stage,native);char *leaf=strrchr(stage,':');if(!leaf){why="invalid native destination";goto done;}
 if((size_t)(leaf-stage)+1+17>255){why="temporary filename exceeds HFS path limit";e=bdNamErr;goto done;}
 why="create temporary file";
 for(unsigned i=0;i<16;i++){snprintf(leaf+1,32,".wget-%08lx-%x",cli_ticks(),i);e=cli_native_create(stage,'BINA','CLRC');if(e!=dupFNErr)break;}
 if(e)goto done;owned=1;e=cli_native_open(stage,0,1,&file);if(e)goto done;
 why="incomplete download";progress_begin();
 while(written<size){int ready=poll_input();if(ready<0)goto done;if(!ready)continue;
  long n=(long)(available-used);if((unsigned long)n>size-written)n=size-written;long want=n;
  e=cli_native_write(file,buffer+used,&n);if(e||n!=want){if(!e)e=ioErr;why="writing download";goto done;}
  used+=n;written+=n;progress_update(written,size,1,0);
 }
 if(app_yield()){cancelled=1;goto done;}
 why="closing download";e=cli_native_close(file);file=-1;if(e)goto done;
 why="committing download";e=cli_native_rename(stage,native);if(e)goto done;owned=0;
 progress_update(written,size,1,1);progress_enabled=0;
 char amount[32];display_size(amount,written);snprintf(header,sizeof header,"Downloaded %s to %s\n",amount,output);say(header);result=0;
done:
 if(progress_enabled){progress_update(written,size,have_length,-1);progress_enabled=0;}
 if(input_failure&&!cancelled)why=input_failure;
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
