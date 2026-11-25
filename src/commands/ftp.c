/* Clarice's small batch FTP client. Independent implementation of RFC 959/2428.
 * Uses the existing native transport and HFS services; no socket emulation. */
#ifdef FTP_HOST_TEST
#include "../../tests/ftp/shim.h"
#else
#include "mac-files.h"
#include "network/native.h"
#endif
extern long cli_read(int,void *,unsigned long);
extern int cli_isatty(int);
#define progress_clock cli_ticks
#define progress_tty() cli_isatty(2)
#define progress_row app_progress
#define progress_columns app_columns
#include "progress.h"
#include "../help-output.h"
static NetConn control,data;
static unsigned char cb[512],buffer[2048];
static unsigned at,have;
static unsigned long activity;
static int cancelled,mac_error;
static const char *failure;
static char reply_text[1024];
static int poll(void){if(app_yield()){cancelled=1;failure="cancelled";return 0;}return 1;}
static int clean(const char *s){for(;*s;s++)if((unsigned char)*s<32||(unsigned char)*s==127)return 0;return 1;}
static int decimal(const char *s,unsigned long *n){unsigned long v=0;if(!*s)return 0;for(;*s;s++){if(*s<'0'||*s>'9'||v>214748364UL||(v==214748364UL&&*s>'7'))return 0;v=v*10+*s-'0';}*n=v;return 1;}
static int line(void){unsigned n=0;int cr=0;for(;;){
 if(!poll())return 0;
 if(at==have){long k=NetAvailable(&control);if(k<0){mac_error=k;failure=k==connectionClosing?"control connection closed":"control status failed";return 0;}
  if(!k){if(cli_ticks()-activity>1800UL){failure="control response timeout";return 0;}continue;}
  if(k>(long)sizeof cb)k=sizeof cb;k=NetRecv(&control,cb,(unsigned short)k,1);
  if(k<=0){if(k<0)mac_error=k;failure="control receive failed";return 0;}at=0;have=k;activity=cli_ticks();}
 unsigned char c=cb[at++];if(c=='\n'&&cr){reply_text[n]=0;return 1;}
 if(cr||(!c)||(c<32&&c!='\r'&&c!='\t')||c==127||n+1>=sizeof reply_text){failure="invalid or oversized FTP reply";return 0;}
 if(c=='\r')cr=1;else reply_text[n++]=c;
}}
static int reply(void){int first=0;unsigned lines=0;unsigned long began=cli_ticks();
 for(;;){if(++lines>256||cli_ticks()-began>1800UL){failure="FTP reply limit exceeded";return -1;}if(!line())return -1;
  size_t n=strlen(reply_text);int code=0;
  if(n>=4&&reply_text[0]>='1'&&reply_text[0]<='5'&&reply_text[1]>='0'&&reply_text[1]<='9'&&reply_text[2]>='0'&&reply_text[2]<='9')code=(reply_text[0]-'0')*100+(reply_text[1]-'0')*10+reply_text[2]-'0';
  if(!first){if(!code||(reply_text[3]!=' '&&reply_text[3]!='-')){failure="malformed FTP reply";return -1;}if(reply_text[3]==' ')return code;first=code;}
  else if(code==first&&reply_text[3]==' ')return code;
 }}
static int command(const char *verb,const char *arg){char out[1024];size_t v=strlen(verb),a=arg?strlen(arg):0;
 if(!poll())return -1;if((arg&&!clean(arg))||v+a+4>sizeof out){failure="invalid or oversized FTP argument";return -1;}
 snprintf(out,sizeof out,arg?"%s %s\r\n":"%s\r\n",verb,arg);mac_error=NetSend(&control,out,strlen(out));memset(out,0,sizeof out);
 if(mac_error){failure="sending FTP command";return -1;}activity=cli_ticks();return reply();}
static int expected(int got,int code){if(got==code)return 1;if(got>=0)failure=reply_text;return 0;}
static int passive(unsigned long ip){int c=command("EPSV",0);unsigned long port=0;char *p,*end;
 if(c==229){p=strchr(reply_text,'(');if(!p||strlen(p)<7||p[2]!=p[1]||p[3]!=p[1])goto bad;
  char delim=p[1];p+=4;end=strchr(p,delim);if(!end||end[1]!=')')goto bad;*end=0;if(!decimal(p,&port))goto bad;
 }else if(c==500||c==501||c==502||c==504||c==522){
  if(!expected(command("PASV",0),227))return 0;p=strchr(reply_text,'(');if(!p)goto bad;p++;unsigned vals[6];
  for(int i=0;i<6;i++){unsigned v=0,n=0;while(*p>='0'&&*p<='9'){if(++n>3)goto bad;v=v*10+*p++-'0';}if(!n||v>255||*p!=(i==5?')':','))goto bad;vals[i]=v;p++;}port=vals[4]*256+vals[5];
 }else{if(c>=0)failure=reply_text;return 0;}
 if(!port||port>65535)goto bad;
 /* Always use the control peer, never a server-supplied third-party address. */
 mac_error=NetConnect(&data,ip,(unsigned short)port);if(mac_error){failure="opening passive data connection";return 0;}return poll();
 bad:failure="invalid passive endpoint";return 0;
}
int clarice_main(int argc,char **argv){
 memset(&control,0,sizeof control);memset(&data,0,sizeof data);at=have=cancelled=mac_error=0;progress_enabled=0;failure="invalid arguments";
 const char *user="anonymous",*host,*remote,*local;char password[512]="clarice@";unsigned short port=21;int from_stdin=0,put=0,owned=0,started=0,result=1,i=1;
 short file=-1;char native[PATH_CAP],canonical[PATH_CAP],stage[PATH_CAP],msg[160];unsigned long ip=0,total=0,done=0;int known=0;CInfoPBRec pb;
 if(argc==2&&!strcmp(argv[1],"--help"))return help_output(1,
  "Usage:\n"
  "  ftp [options] get HOST REMOTE NEW_LOCAL\n"
  "  ftp [options] put HOST LOCAL REMOTE\n"
  "Transfer data forks over batch binary FTP. Use MacBinary for complete Macintosh files.\n"
  "Options:\n"
  "  -u USER\tLog in as USER. Default: anonymous.\n"
  "  -P PORT\tConnect to PORT. Default: 21.\n"
  "  --password-stdin\tRead one password line (up to 511 bytes). Keep secrets out of shell history.\n"
  "Downloads refuse existing files and commit after server success.\n"
  "Uploads can overwrite remote files. Failed uploads may leave partial files.\n"
  "Uses system DNS and passive EPSV/PASV. No TLS, active mode, resume, recursion or interactive commands.\n"
  "Idle timeout: 30 seconds. DNS and pending callbacks can delay Ctrl+C.\n"
  "Status: 0 success, 1 failure, 2 usage, 130 cancelled.\n");
 while(i<argc&&argv[i][0]=='-'){
  if(!strcmp(argv[i],"--password-stdin"))from_stdin=1;
  else if(!strcmp(argv[i],"-u")&&i+1<argc)user=argv[++i];
  else if(!strcmp(argv[i],"-P")&&i+1<argc){unsigned long n;if(!decimal(argv[++i],&n)||!n||n>65535){result=2;goto done;}port=n;}
  else{result=2;goto done;}i++;
 }
 if(argc-i!=4||(!strcmp(argv[i],"get")?0:strcmp(argv[i],"put"))){result=2;goto done;}
 put=!strcmp(argv[i++],"put");host=argv[i++];local=argv[i+(put?0:1)];remote=argv[i+(put?1:0)];
 if(!*remote||!clean(remote)||strlen(remote)>900||!*user||!clean(user)||strlen(user)>500){result=2;goto done;}
 if(strcmp(user,"anonymous")&&!from_stdin){failure="non-anonymous login requires --password-stdin";result=2;goto done;}
 if(from_stdin){unsigned n=0;for(;;){char c;long k=cli_read(0,&c,1);if(!poll())goto done;if(k<0){failure="reading password";goto done;}if(!k||c=='\n')break;if(n+1>=sizeof password){failure="password too long";goto done;}password[n++]=c;}if(n&&password[n-1]=='\r')n--;password[n]=0;if(!clean(password)){failure="invalid password";goto done;}}
 failure="local path";mac_error=checked(local,native,canonical,put);if(mac_error)goto done;
 mac_error=info(native,&pb);
 if(put){if(mac_error||isdir(&pb)){if(!mac_error)mac_error=ioErr;goto done;}total=(unsigned long)pb.hFileInfo.ioFlLgLen;known=1;mac_error=cli_native_open(native,0,0,&file);if(mac_error)goto done;}
 else{if(!mac_error){failure="destination exists (not overwritten)";goto done;}if(mac_error!=fnfErr)goto done;mac_error=0;}
 failure="system hostname resolution";mac_error=NetResolve(host,&ip,app_yield,1800);if(mac_error){if(mac_error==userCanceledErr)cancelled=1;goto done;}
 failure="control connection";mac_error=NetConnect(&control,ip,port);if(mac_error)goto done;activity=cli_ticks();
 int c=reply();if(c==120)c=reply();if(!expected(c,220))goto done;
 c=command("USER",user);if(c==331)c=command("PASS",password);memset(password,0,sizeof password);if(!expected(c,230)||!expected(command("TYPE","I"),200))goto done;
 if(!put){c=command("SIZE",remote);if(c==213){if(!decimal(reply_text+4,&total)){failure="invalid SIZE reply";goto done;}known=1;}else if(c<0||c==421){if(c==421)failure=reply_text;goto done;}}
 if(!passive(ip))goto done;
 /* STOR may truncate remotely before its preliminary reply reaches us. */
 if(put)started=1;
 c=command(put?"STOR":"RETR",remote);if(c!=125&&c!=150){if(c>=0)failure=reply_text;goto done;}started=1;
 if(!put){strcpy(stage,native);char *leaf=strrchr(stage,':');if(!leaf||(size_t)(leaf-stage)+20>=255){failure="temporary path too long";goto done;}
  failure="creating temporary file";for(unsigned j=0;j<16;j++){snprintf(leaf+1,20,".ftp-%08lx-%x",cli_ticks(),j);mac_error=cli_native_create(stage,'BINA','CLRC');if(mac_error!=dupFNErr)break;}if(mac_error)goto done;owned=1;mac_error=cli_native_open(stage,0,1,&file);if(mac_error)goto done;
 }
 progress_begin();activity=cli_ticks();failure="data transfer";
 for(;;){if(!poll())goto done;long n=sizeof buffer;
  if(put){mac_error=cli_native_read(file,buffer,&n);if(mac_error&&mac_error!=eofErr)goto done;mac_error=0;if(!n)break;if((unsigned long)n>total-done){failure="local file changed during upload";goto done;}mac_error=NetSend(&data,buffer,n);if(mac_error)goto done;}
  else{n=NetAvailable(&data);if(n==connectionClosing)break;if(n<0){mac_error=n;failure="data status failed";goto done;}if(!n){if(cli_ticks()-activity>1800UL){failure="data timeout";goto done;}continue;}if(n>(long)sizeof buffer)n=sizeof buffer;n=NetRecv(&data,buffer,n,1);if(n==connectionClosing)break;if(n<=0){if(n<0)mac_error=n;failure="data receive failed";goto done;}
   if((unsigned long)n>2147483647UL-done||(known&&((unsigned long)n>total-done))){failure="download exceeds declared/platform size";goto done;}long want=n;mac_error=cli_native_write(file,buffer,&n);if(mac_error||n!=want){if(!mac_error)mac_error=ioErr;goto done;}}
  done+=n;activity=cli_ticks();progress_update(done,total,known,0);
 }
 if(known&&done!=total){failure="incomplete transfer (size mismatch)";goto done;}
 failure="closing data connection";mac_error=NetClose(&data);if(mac_error)goto done;
 activity=cli_ticks();c=reply();if(c!=226&&c!=250){if(c>=0)failure=reply_text;goto done;}
 if(!poll())goto done;failure="closing local file";mac_error=cli_native_close(file);file=-1;if(mac_error)goto done;
 if(!put){failure="committing download";mac_error=cli_native_rename(stage,native);if(mac_error)goto done;owned=0;}
 progress_update(done,total,known,1);progress_enabled=0;result=0;
 done:
 memset(password,0,sizeof password);
 if(progress_enabled){progress_update(done,total,known,-1);progress_enabled=0;}
 if(file>=0){int e=cli_native_close(file);if(e&&!mac_error)mac_error=e;}
 int de=NetClose(&data),ce=NetClose(&control);if(de||ce){errsay("ftp: network release failed; driver buffer retained for safety\n");if(!result){result=1;failure="transfer finished but network cleanup failed";}}
 if(owned){int e=cli_native_delete(stage);if(e){errsay("ftp: cleanup failed; recover temporary file at native:");errsay(stage);errsay("\n");}}
 if(result){errsay("ftp: ");errsay(cancelled?"cancelled":failure);if(mac_error){snprintf(msg,sizeof msg," (Mac error %d)",mac_error);errsay(msg);}errsay("\n");if(put&&started){errsay("ftp: remote upload may be partial: ");errsay(remote);errsay("\n");}}
 else{snprintf(msg,sizeof msg,"%s %lu bytes\n",put?"Uploaded":"Downloaded",done);say(msg);}
 return cancelled?130:result;
}
