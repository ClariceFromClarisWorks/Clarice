/* Optional native MacTCP service. Protocol: unchanged upstream libtelnet.
 * No notify callbacks: the event loop polls stable async parameter blocks. */
#include <Multiverse.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../session.h"
#include "../../service.h"
#include "../network/telnet-engine.h"
#include "../vendor/mactcp/nettcp.c"
extern long cli_write(int,const void *,unsigned long);
extern int app_yield(void);
#define LINE 1024
#define QUEUE 4096
/* All memory referenced by MacTCP survives application teardown until release. */
typedef struct {
 TCPiopb opening,sending;
 struct {unsigned short length;Ptr ptr;unsigned short end;} wds;
 unsigned char ring[16384],send[QUEUE];
} DriverStorage;
static DriverStorage *driver;
static Ptr stream;
static telnet_t *engine;
static void *session;
static unsigned long peer;
static unsigned short port;
static int connected,running,failed,interrupted,send_pending,opening_pending,closing;
static unsigned long send_tick,last_pump;
static char output[QUEUE],line[LINE],ready[QUEUE],previous[LINE];
static unsigned used,line_used,line_cursor,ready_used,ready_at;
static int bad_line,after_cr,escape,eof_pending;
static void pump(void);
#include "auth.inc"
static void prompt(void){char p[132];cli_session_prompt(session,p,sizeof p);telnet_send_text(engine,p,strlen(p));}
static void text(const char *s){if(engine)telnet_send_text(engine,s,strlen(s));}
static void echo_text(const char *s){if(engine&&clarice_telnet_enabled(engine,TELNET_TELOPT_ECHO,1))text(s);}
/* NVT backspace redraw works without claiming a full-screen terminal. */
static void line_echo(const char *s,unsigned n){
 if(!engine||!clarice_telnet_enabled(engine,TELNET_TELOPT_ECHO,1))return;
 while(n){const char *tab=memchr(s,'\t',n);unsigned count=tab?(unsigned)(tab-s):n;
  if(count)telnet_send(engine,s,count);if(!tab)break;
  telnet_send(engine," ",1);s+=count+1;n-=count+1;
 }
}
static void line_left(unsigned n){while(n--)echo_text("\b");}
static void line_tail(void){
 line_echo(line+line_cursor,line_used-line_cursor);
 line_left(line_used-line_cursor);
}
static void line_clear(void){
 line_left(line_cursor);for(unsigned i=0;i<line_used;i++)echo_text(" ");line_left(line_used);line_used=line_cursor=0;
}
static void interrupt(void){if(protected_login&&!session){wipe(password,sizeof password);failed=1;return;}interrupted=1;line_used=line_cursor=ready_used=ready_at=bad_line=escape=0;text("^C\n");if(!running){interrupted=0;prompt();}}
static void character(unsigned char c){
 if(closing)return;
 if(protected_login&&!session){
  if(auth_pending)return;
  if(c==3||c==4){wipe(password,sizeof password);failed=1;return;}
  if(c=='\r'||c=='\n'){password[auth_length]=0;auth_pending=1;after_cr=c=='\r';return;}
  if(c==8||c==127){if(auth_length)password[--auth_length]=0;return;}
  if(c==21){wipe(password,sizeof password);auth_length=0;return;}
  if(c<32||c>126)return;
  if(auth_length>=sizeof password-1){auth_overflow=1;return;}
  password[auth_length++]=c;return;
 }
 if(c==3){interrupt();return;}
 if(after_cr){after_cr=0;if(c=='\n'||c==0)return;}
 if(c=='\r'||c=='\n'){
  after_cr=c=='\r';echo_text("\n");
  if(ready_at){memmove(ready,ready+ready_at,ready_used-ready_at);ready_used-=ready_at;ready_at=0;}
  if(bad_line||line_used+1>QUEUE-ready_used){text("Input rejected: pending command capacity exceeded.\n");bad_line=0;line_used=line_cursor=0;if(!running)prompt();return;}
  memcpy(ready+ready_used,line,line_used);ready_used+=line_used;ready[ready_used++]='\n';line_used=line_cursor=escape=0;return;
 }
 if(bad_line)return;
 if(escape){
  if(escape==1&&(c=='['||c=='O')){escape=2;return;}
  if(escape==2){
   if(c=='D'){if(line_cursor){line_cursor--;line_left(1);}}
   else if(c=='C'){if(line_cursor<line_used){line_echo(line+line_cursor,1);line_cursor++;}}
   else if(c=='H'){line_left(line_cursor);line_cursor=0;}
   else if(c=='F'){line_echo(line+line_cursor,line_used-line_cursor);line_cursor=line_used;}
   else if(c=='A'||c=='B'){line_clear();if(c=='A'){strcpy(line,previous);line_used=line_cursor=strlen(line);line_echo(line,line_used);}}
   else if(c>=0x20&&c<0x40)return; /* consume bounded CSI parameters */
  }escape=0;return;
 }
 if(c==27){escape=1;return;}
 if(c==4&&!line_used){if(running)eof_pending=1;else failed=1;return;}
 if(c==8||c==127){if(line_cursor){line_cursor--;line_used--;memmove(line+line_cursor,line+line_cursor+1,line_used-line_cursor);line_left(1);line_echo(line+line_cursor,line_used-line_cursor);echo_text(" ");line_left(line_used-line_cursor+1);}return;}
 if(c==21){line_clear();return;}
 if(c<32&&c!='\t')return;
 if(line_used>=LINE-1){bad_line=1;return;}
 memmove(line+line_cursor+1,line+line_cursor,line_used-line_cursor);line[line_cursor++]=c;line_used++;line_echo((char*)&c,1);line_tail();
}
static void event(telnet_t *t,telnet_event_t *e,void *ctx){(void)t;(void)ctx;
 switch(e->type){
 case TELNET_EV_SEND:if(e->data.size>QUEUE-used){failed=1;break;}memcpy(output+used,e->data.buffer,e->data.size);used+=e->data.size;break;
 case TELNET_EV_DATA:for(size_t i=0;i<e->data.size;i++)character((unsigned char)e->data.buffer[i]);break;
 case TELNET_EV_IAC:if(e->iac.cmd==TELNET_IP||e->iac.cmd==TELNET_BREAK)interrupt();else if(e->iac.cmd==TELNET_EC)character(8);else if(e->iac.cmd==TELNET_EL)character(21);break;
 case TELNET_EV_ERROR:case TELNET_EV_WARNING:failed=1;break;
 default:break;
 }
}
static long read_session(void *ctx,void *p,unsigned long n){(void)ctx;
 while(!ready_used&&!eof_pending&&!failed&&!interrupted){pump();if(app_yield())interrupted=1;}
 if(failed||interrupted)return -1;
 if(eof_pending&&!ready_used){eof_pending=0;return 0;}
 unsigned count=ready_used-ready_at;if(n<count)count=n;memcpy(p,ready+ready_at,count);ready_at+=count;
 if(ready_at==ready_used)ready_at=ready_used=0;return count;
}
static long write_session(void *ctx,int fd,const void *p,unsigned long n){(void)ctx;(void)fd;unsigned long done=0;
 while(done<n&&!failed&&!interrupted){
  while(used>QUEUE-512&&!failed&&!interrupted){pump();if(app_yield())interrupted=1;}
  unsigned count=n-done;if(count>128)count=128;
  telnet_send_text(engine,(const char*)p+done,count);done+=count;pump();if(app_yield())interrupted=1;
 }
 return done==n?(long)n:-1;
}
static int cancel_session(void *ctx){(void)ctx;pump();return failed||interrupted;}
static void pb_init(TCPiopb *p,short code){memset(p,0,sizeof *p);p->ioCRefNum=gIPPRefNum;p->csCode=code;p->tcpStream=stream;}
/* Return only after the driver no longer owns any buffers. Failure keeps the
 * service image and system storage alive; remote off can retry safely. */
static int release_stream(void){TCPiopb p;OSErr e;
 if(stream){pb_init(&p,TCPAbort);PBControlSync((ParmBlkPtr)&p);
  if((opening_pending&&(*(volatile short*)&driver->opening.ioResult)==1)||(send_pending&&(*(volatile short*)&driver->sending.ioResult)==1))return -1;
  pb_init(&p,TCPRelease);e=PBControlSync((ParmBlkPtr)&p);if(e)return e;stream=0;
 }
 opening_pending=send_pending=connected=0;return 0;
}
static int listen_again(void){TCPiopb p;pb_init(&p,TCPCreate);p.csParam.create.rcvBuff=(Ptr)driver->ring;p.csParam.create.rcvBuffLen=sizeof driver->ring;
 OSErr e=PBControlSync((ParmBlkPtr)&p);if(e)return e;stream=p.tcpStream;
 pb_init(&driver->opening,TCPPassiveOpen);driver->opening.csParam.open.localPort=port;
 driver->opening.csParam.open.remoteHost=peer; /* explicit trusted peer only */
 e=PBControlAsync((ParmBlkPtr)&driver->opening);if(e)return e;opening_pending=1;return 0;
}
static void pump(void){
 if(!driver||failed)return;
 unsigned long now=TickCount();if(now==last_pump)return;last_pump=now;
 if(closing==2){if((*(volatile short*)&driver->opening.ioResult)==1)return;opening_pending=0;closing=0;failed=1;return;}
 if(opening_pending){if((*(volatile short*)&driver->opening.ioResult)==1)return;opening_pending=0;if(driver->opening.ioResult){failed=1;return;}connected=1;
  if(driver->opening.csParam.open.remoteHost!=peer){failed=1;return;}
  static const telnet_telopt_t options[]={{TELNET_TELOPT_ECHO,TELNET_WILL,TELNET_DONT},{TELNET_TELOPT_SGA,TELNET_WILL,TELNET_DO},{-1,0,0}};
  engine=telnet_init(options,event,0,0);if(!engine){failed=1;return;}
  if(!protected_login){ClariceSessionIO io={0,read_session,write_session,cancel_session};session=cli_session_open(&io);if(!session){failed=1;return;}}
  telnet_negotiate(engine,TELNET_WILL,TELNET_TELOPT_ECHO);telnet_negotiate(engine,TELNET_WILL,TELNET_TELOPT_SGA);
  text("Clarice remote shell (unencrypted; one session)\nLine editing: arrows/Home/End, Backspace, Ctrl-U; Up recalls last command.\nTab is whitespace (no completion). Reconnect after disconnect cleanup.\n");
  if(protected_login){auth_started=now;if(auth_retry&&(long)(now-auth_retry)<0){failed=1;return;}text("Password: ");}else prompt();
 }
 if(!connected)return;
 if(protected_login&&!session&&now-auth_started>3600){wipe(password,sizeof password);failed=1;return;}
 if(send_pending){if((*(volatile short*)&driver->sending.ioResult)==1){if(TickCount()-send_tick>600)failed=1;}else{send_pending=0;if(driver->sending.ioResult)failed=1;}}
 if(!send_pending&&used&&!failed){memcpy(driver->send,output,used);driver->wds.length=used;driver->wds.ptr=(Ptr)driver->send;driver->wds.end=0;used=0;
  pb_init(&driver->sending,TCPSend);driver->sending.csParam.send.wdsPtr=(Ptr)&driver->wds;driver->sending.csParam.send.pushFlag=1;
  driver->sending.csParam.send.ulpTimeoutValue=5;driver->sending.csParam.send.ulpTimeoutAction=1;driver->sending.csParam.send.validityFlags=0xc0;
  if(PBControlAsync((ParmBlkPtr)&driver->sending))failed=1;else{send_pending=1;send_tick=TickCount();}
 }
 if(closing&&!used&&!send_pending){
  pb_init(&driver->opening,TCPClose);driver->opening.csParam.close.ulpTimeoutValue=5;driver->opening.csParam.close.ulpTimeoutAction=1;driver->opening.csParam.close.validityFlags=0xc0;
  if(PBControlAsync((ParmBlkPtr)&driver->opening))failed=1;else{closing=2;opening_pending=1;}return;
 }
 TCPiopb p;pb_init(&p,TCPStatus);if(PBControlSync((ParmBlkPtr)&p)){failed=1;return;}
 unsigned long available=p.csParam.status.amtUnreadData;
 if(!available){if(p.csParam.status.connectionState!=8)failed=1;return;}
 char bytes[256];if(available>sizeof bytes)available=sizeof bytes;
 pb_init(&p,TCPRcv);p.csParam.receive.rcvBuff=bytes;p.csParam.receive.rcvBuffLen=available;p.csParam.receive.commandTimeoutValue=1;
 if(PBControlSync((ParmBlkPtr)&p)){failed=1;return;}telnet_recv(engine,bytes,p.csParam.receive.rcvBuffLen);
}
static int stop(void){if(running)return -1;if(release_stream())return -1;
 if(session){cli_session_close(session);session=0;}if(engine){telnet_free(engine);engine=0;}
 if(driver){DisposePtr((Ptr)driver);driver=0;}wipe(password,sizeof password);wipe(verifier,sizeof verifier);return 0;
}
static void poll_service(int execute){
 pump();
 if(failed&&!running){if(release_stream())return;if(session){cli_session_close(session);session=0;}if(engine){telnet_free(engine);engine=0;}
  used=line_used=line_cursor=ready_used=ready_at=bad_line=after_cr=escape=interrupted=eof_pending=closing=0;previous[0]=0;wipe(password,sizeof password);auth_pending=auth_length=auth_overflow=0;failed=0;if(listen_again())failed=1;return;}
 if(execute&&auth_pending&&!session&&!failed){
  unsigned char result[32];int e=derive(password,result);unsigned difference=0;for(int i=0;i<32;i++)difference|=result[i]^verifier[i];wipe(result,sizeof result);wipe(password,sizeof password);
  auth_pending=0;if(e||difference||auth_overflow||!auth_length){text("\nAuthentication failed.\n");auth_retry=TickCount()+300;closing=1;return;}
  auth_length=0;ClariceSessionIO io={0,read_session,write_session,cancel_session};session=cli_session_open(&io);if(!session){failed=1;return;}text("\n");prompt();
 }
 if(execute&&session&&ready_used&&!running&&!failed&&!closing){char command[LINE];unsigned n=0;while(n<ready_used&&ready[n]!='\n')n++;if(n>=LINE){failed=1;return;}memcpy(command,ready,n);command[n]=0;
  strcpy(previous,command);unsigned consumed=n<ready_used?n+1:n;memmove(ready,ready+consumed,ready_used-consumed);ready_used-=consumed;ready_at=0;running=1;interrupted=0;int rc=cli_session_run(session,command);running=0;
  if(rc&256)closing=1;else if(!failed){interrupted=0;prompt();}pump();
 }
}
int clarice_main(int argc,char **argv){int explicit_no_password=0;port=2323;
 if(argc==2&&!strcmp(argv[1],"--preferences")){int rc=configure();if(rc!=3)return rc;goto start;}
 if(argc==1){ClariceRemoteSettings settings;memset(&settings,0,sizeof settings);if(config_load(&settings)){const char *m="telnetd: missing/invalid password settings; use Preferences > Telnet server.\n";cli_write(2,m,strlen(m));return 1;}peer=NetParseIP(settings.peer);port=settings.port;protected_login=1;goto start;}
 for(int i=1;i<argc;i++){
  if(!strcmp(argv[i],"--no-password"))explicit_no_password=1;
  else if(!strcmp(argv[i],"--peer")&&i+1<argc)peer=NetParseIP(argv[++i]);
  else if(!strcmp(argv[i],"--port")&&i+1<argc){char *end;unsigned long n=strtoul(argv[++i],&end,10);if(*end||!n||n>65535)return 2;port=n;}
  else{const char *help="telnetd [--preferences] | --no-password --peer IPv4 [--port N]\nOne supplementary plain-text shell; trusted peer required.\nBare telnetd requires saved password settings. remote off stops it.\nPassword settings: Preferences > Telnet server, or telnetd --preferences.\n1023-byte lines, 4 KiB pending input, arrows/Home/End, last-command Up/Down; overflow rejected.\nTab is whitespace, not completion. Immediate reconnect may require retry.\nSettings: /etc/clarice/telnet.conf (salted verifier, never plaintext).\nPasswords: 1-127 ASCII bytes; 60-second login timeout, 5-second failure cooldown.\nGUI-only commands/editors remain local. Status: 0 success, 1 failure, 2 usage, 130 cancelled.\n";cli_write(1,help,strlen(help));return !strcmp(argv[i],"--help")?0:2;}
 }
 if(!peer||!explicit_no_password){const char *m="telnetd: require --no-password and --peer IPv4 (trusted local network only)\n";cli_write(2,m,strlen(m));return 2;}
 start:;
 OSErr start_error=NetInit();if(start_error){char message[128];snprintf(message,sizeof message,"telnetd: %s (Mac OS %d)\n",NetErrStr(start_error),start_error);cli_write(2,message,strlen(message));return 1;}driver=(DriverStorage*)NewPtrSysClear(sizeof *driver);if(!driver)return 1;
 if(listen_again()){if(!release_stream()){DisposePtr((Ptr)driver);driver=0;}return 1;}
 ClariceService service={poll_service,stop};if(cli_service_register(&service)){stop();return 1;}
 const char *m="Telnet enabled for the specified peer. Unencrypted; use remote off to stop.\n";cli_write(1,m,strlen(m));return 0;
}
