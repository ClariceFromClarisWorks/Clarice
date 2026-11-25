/* Optional native MacTCP service. Protocol: unchanged upstream libtelnet.
 * No notify callbacks: the event loop polls stable async parameter blocks. */
#include <Multiverse.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../session.h"
#include "../../service.h"
#include "../../remote-settings.h"
#ifdef REMOTE_SERIAL
#include <Serial.h>
#include <Devices.h>
#define SERVICE_SLOT 1
#define SERVICE_NAME "seriald"
#define AUTH_PATH "/etc/clarice/serial.conf"
#define AUTH_MAGIC "CLARICE-SERIAL-1"
typedef void telnet_t;
static unsigned long NetParseIP(const char *);
static int NetInit(void);
static const char *NetErrStr(int);
static void queue_bytes(const char *,unsigned long);
static void serial_text(const char *,unsigned long);
#define telnet_send(t,p,n) queue_bytes(p,n)
#define telnet_send_text(t,p,n) serial_text(p,n)
#define clarice_telnet_enabled(t,o,d) 1
#define telnet_free(t) ((void)0)
#define TELNET_TELOPT_ECHO 1
#else
#define SERVICE_SLOT 0
#define SERVICE_NAME "telnetd"
#include "../network/telnet-engine.h"
#include "../vendor/mactcp/nettcp.c"
#endif
extern long cli_write(int,const void *,unsigned long);
extern int app_yield(void);
#define LINE 1024
#define QUEUE 4096
/* All memory referenced by MacTCP survives application teardown until release. */
typedef struct {
#ifdef REMOTE_SERIAL
 ParamBlockRec sending;short in,out;int buffer_set;
#else
 TCPiopb opening,sending;
 struct {unsigned short length;Ptr ptr;unsigned short end;} wds;
#endif
#ifdef REMOTE_SERIAL
 unsigned char ring[4096],send[QUEUE];
#else
 unsigned char ring[16384],send[QUEUE];
#endif
} DriverStorage;
static DriverStorage *driver;
static Ptr stream;
static telnet_t *engine;
static void *session;
static unsigned long peer;
static unsigned short port;
static int connected,running,failed,interrupted,send_pending,opening_pending,closing;
static unsigned long send_tick,last_pump,retry_tick;
static int listener_error;
static char output[QUEUE],line[LINE],ready[QUEUE],previous[LINE];
static unsigned used,line_used,line_cursor,ready_used,ready_at;
/* Retain a bounded native receive batch while amplified echo drains. Native
 * TCP receive-window backpressure owns the rest; no recursive event pumping. */
static char received[256];
static unsigned received_at,received_count;
static int bad_line,after_cr,escape,eof_pending,prompt_pending;
extern void *cli_alloc(unsigned long);
extern void cli_free(void *);
static unsigned char *binary_input;
static unsigned binary_count,binary_at;
static long session_wire(int,void *,unsigned long);
static void pump(void);
#include "auth.inc"
/* BINARY negotiates an eight-bit wire, not a change to shell newline display.
   Keep terminal text CRLF; raw transfer output uses telnet_send separately. */
static void terminal_text(const char *p,unsigned long n){
#ifndef REMOTE_SERIAL
 if(clarice_telnet_enabled(engine,TELNET_TELOPT_BINARY,1)){
  const char *start=p;unsigned long span=0;
  while(n--){if(*p=='\n'){if(span)telnet_send(engine,start,span);telnet_send(engine,"\r\n",2);start=p+1;span=0;}else span++;p++;}
  if(span)telnet_send(engine,start,span);return;
 }
#endif
 telnet_send_text(engine,p,n);
}
static void prompt(void){prompt_pending=1;}
static void send_prompt(void){if(prompt_pending&&!used&&engine&&session&&!failed){char p[132];cli_session_prompt(session,p,sizeof p);terminal_text(p,strlen(p));prompt_pending=0;}}
static void text(const char *s){if(engine)terminal_text(s,strlen(s));}
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
/* Replace history in one pass: even IAC-escaped bytes fit the 4 KiB queue. */
static void line_history(int recall){
 unsigned old=line_used;line_left(line_cursor);line_used=line_cursor=0;
 if(recall){strcpy(line,previous);line_used=line_cursor=strlen(line);line_echo(line,line_used);}
 if(old>line_used){for(unsigned i=line_used;i<old;i++)echo_text(" ");line_left(old-line_used);}
}
static void line_insert(const char *p,unsigned n){
 unsigned room=LINE-1-line_used;if(n>room){n=room;bad_line=1;}
 memmove(line+line_cursor+n,line+line_cursor,line_used-line_cursor);
 memcpy(line+line_cursor,p,n);line_cursor+=n;line_used+=n;line_echo(p,n);line_tail();
}
static void interrupt(void){if(binary_input){interrupted=1;return;}if(protected_login&&!session){wipe(password,sizeof password);failed=1;return;}interrupted=1;line_used=line_cursor=ready_used=ready_at=bad_line=escape=0;text("^C\n");if(!running){interrupted=0;prompt();}}
static void character(unsigned char c){
 if(closing)return;
 if(binary_input){if(c==3){interrupted=1;return;}if(binary_count==QUEUE){failed=1;return;}binary_input[(binary_at+binary_count)%QUEUE]=c;binary_count++;return;}
 if(protected_login&&!session){
  if(!auth_pending&&after_cr){after_cr=0;if(c=='\n'||c==0)return;}
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
   else if(c=='A'||c=='B')line_history(c=='A');
   else if(c>=0x20&&c<0x40)return; /* consume bounded CSI parameters */
  }escape=0;return;
 }
 if(c==27){escape=1;return;}
 if(c==4&&!line_used){if(running)eof_pending=1;else failed=1;return;}
 if(c==8||c==127){if(line_cursor){line_cursor--;line_used--;memmove(line+line_cursor,line+line_cursor+1,line_used-line_cursor);line_left(1);line_echo(line+line_cursor,line_used-line_cursor);echo_text(" ");line_left(line_used-line_cursor+1);}return;}
 if(c==21){line_clear();return;}
 if(c<32&&c!='\t')return;
 if(line_used>=LINE-1){bad_line=1;return;}
 line_insert((char*)&c,1);
}
/* Coalesce printable paste runs: redraw the tail once, not once per byte. */
static int printable(unsigned char c){return c>=32&&c!=127&&c!=255;}
static void characters(const char *p,unsigned n){
 while(n){unsigned k=0;
  if(!binary_input&&(!protected_login||session)&&!closing&&!bad_line&&!escape)
   while(k<n&&printable((unsigned char)p[k]))k++;
  if(k){after_cr=0;line_insert(p,k);p+=k;n-=k;}else{character((unsigned char)*p++);n--;}
 }
}
/* One action needs at most 4092 echo bytes (including doubled IAC). Drain
 * existing output first. Escape/protocol bytes stay with the upstream parser. */
static void receive_pending(void){
 while(received_at<received_count&&!used&&!failed){unsigned n=1;
  if(!escape&&printable((unsigned char)received[received_at]))
   while(received_at+n<received_count&&printable((unsigned char)received[received_at+n]))n++;
#ifdef REMOTE_SERIAL
  characters(received+received_at,n);
#else
  telnet_recv(engine,received+received_at,n);
#endif
  received_at+=n;
 }
 if(received_at==received_count)received_at=received_count=0;
}
#ifndef REMOTE_SERIAL
static void event(telnet_t *t,telnet_event_t *e,void *ctx){(void)t;(void)ctx;
 switch(e->type){
 case TELNET_EV_SEND:if(e->data.size>QUEUE-used){failed=1;break;}memcpy(output+used,e->data.buffer,e->data.size);used+=e->data.size;break;
 case TELNET_EV_DATA:characters(e->data.buffer,e->data.size);break;
 case TELNET_EV_IAC:if(e->iac.cmd==TELNET_IP||e->iac.cmd==TELNET_BREAK)interrupt();else if(e->iac.cmd==TELNET_EC)character(8);else if(e->iac.cmd==TELNET_EL)character(21);break;
 case TELNET_EV_ERROR:case TELNET_EV_WARNING:failed=1;break;
 default:break;
 }
}
#endif
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
  terminal_text((const char*)p+done,count);done+=count;pump();if(app_yield())interrupted=1;
 }
 return done==n?(long)n:-1;
}
static int cancel_session(void *ctx){(void)ctx;pump();return failed||interrupted;}
#ifdef REMOTE_SERIAL
#include "serial-backend.inc"
#else
static void pb_init(TCPiopb *p,short code){memset(p,0,sizeof *p);p->ioCRefNum=gIPPRefNum;p->csCode=code;p->tcpStream=stream;}
/* Return only after the driver no longer owns any buffers. Failure keeps the
 * service image and system storage alive; remote off can retry safely. */
static int release_stream(void){TCPiopb p;OSErr e;
 if(stream){pb_init(&p,TCPAbort);PBControlSync((ParmBlkPtr)&p);
  if((opening_pending&&(*(volatile short*)&driver->opening.ioResult)==1)||(send_pending&&(*(volatile short*)&driver->sending.ioResult)==1))return -1;
  pb_init(&p,TCPRelease);e=PBControlSync((ParmBlkPtr)&p);if(e)return e;stream=0;
 }
 opening_pending=send_pending=connected=0;app_remote_indicator(SERVICE_SLOT,0);return 0;
}
#ifdef SE_NETWORK_DIAGNOSTICS
/* Bounded development evidence; absent from normal builds. Never log credentials. */
static void se_network_diag(const char *phase,int error){
 static unsigned count;
 if(count++<12){char message[128];int n=snprintf(message,sizeof message,"SE TCP %s: %d at %lu ticks\n",phase,error,TickCount());cli_write(2,message,n);}
}
#else
#define se_network_diag(phase,error) ((void)0)
#endif
static int listen_again(void){TCPiopb p;pb_init(&p,TCPCreate);p.csParam.create.rcvBuff=(Ptr)driver->ring;p.csParam.create.rcvBuffLen=sizeof driver->ring;
 OSErr e=PBControlSync((ParmBlkPtr)&p);if(e)return e;stream=p.tcpStream;
 pb_init(&driver->opening,TCPPassiveOpen);driver->opening.csParam.open.localPort=port;
 /* Native partial wildcard (fixed host, port zero) refuses ordinary clients
    on the physical SE. Listen with both remote fields zero, then check the
    returned peer before any Telnet/authentication work. No extra allocation. */
#ifdef SE_NETWORK_FIXED_PEER_PORT
 driver->opening.csParam.open.remoteHost=peer;
 driver->opening.csParam.open.remotePort=23230; /* historical matching diagnostic */
#else
 driver->opening.csParam.open.remoteHost=0; /* remotePort already zero via pb_init */
#endif
 e=PBControlAsync((ParmBlkPtr)&driver->opening);se_network_diag("passive-submit",e);se_network_diag("passive-result",driver->opening.ioResult);if(e)return e;opening_pending=1;return 0;
}
static void pump(void){
 if(!driver||failed)return;
 unsigned long now=TickCount();if(now==last_pump)return;last_pump=now;
 if(closing==2){if((*(volatile short*)&driver->opening.ioResult)==1)return;opening_pending=0;closing=0;failed=1;return;}
 if(opening_pending){if((*(volatile short*)&driver->opening.ioResult)==1)return;opening_pending=0;se_network_diag("passive-completion",driver->opening.ioResult);if(driver->opening.ioResult){failed=1;return;}
  if(driver->opening.csParam.open.remoteHost!=peer){failed=1;return;}
  connected=1;app_remote_indicator(SERVICE_SLOT,1);
  static const telnet_telopt_t options[]={{TELNET_TELOPT_BINARY,TELNET_WILL,TELNET_DO},{TELNET_TELOPT_ECHO,TELNET_WILL,TELNET_DONT},{TELNET_TELOPT_SGA,TELNET_WILL,TELNET_DO},{-1,0,0}};
  engine=telnet_init(options,event,0,0);if(!engine){failed=1;return;}
  if(!protected_login){ClariceSessionIO io={0,read_session,write_session,cancel_session};session=cli_session_open(&io);if(session)cli_session_set_wire(session,session_wire);if(!session){failed=1;return;}}
  telnet_negotiate(engine,TELNET_WILL,TELNET_TELOPT_BINARY);telnet_negotiate(engine,TELNET_DO,TELNET_TELOPT_BINARY);
  telnet_negotiate(engine,TELNET_WILL,TELNET_TELOPT_ECHO);telnet_negotiate(engine,TELNET_WILL,TELNET_TELOPT_SGA);
  text("Clarice remote shell (unencrypted; one session)\nLine editing: arrows/Home/End, Backspace, Ctrl-U; Up recalls last command.\nTab is whitespace (no completion). Reconnect after disconnect cleanup.\n");
  if(protected_login){auth_started=now;if(auth_retry&&(long)(now-auth_retry)<0){failed=1;return;}text("Password: ");}else prompt();
 }
 if(!connected)return;
 if(protected_login&&!session&&now-auth_started>3600){wipe(password,sizeof password);failed=1;return;}
 send_prompt();
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
 receive_pending();if(received_count||used)return;
 TCPiopb p;pb_init(&p,TCPStatus);if(PBControlSync((ParmBlkPtr)&p)){failed=1;return;}
 unsigned long available=p.csParam.status.amtUnreadData;
 if(!available){if(p.csParam.status.connectionState!=8)failed=1;return;}
 if(available>sizeof received)available=sizeof received;
 pb_init(&p,TCPRcv);p.csParam.receive.rcvBuff=received;p.csParam.receive.rcvBuffLen=available;p.csParam.receive.commandTimeoutValue=1;
 if(PBControlSync((ParmBlkPtr)&p)){failed=1;return;}received_count=p.csParam.receive.rcvBuffLen;receive_pending();
}
#endif
/* Raw bytes are owned only for a foreground transfer. LibTelnet still handles
   IAC framing; line editing/echo and ordinary text output must not touch payload. */
static long session_wire(int op,void *data,unsigned long size){
 if(op==0)return 1;
 if(op==1){if(binary_input||!running||failed||ready_used||line_used)return -1;
#ifndef REMOTE_SERIAL
  if(!clarice_telnet_enabled(engine,TELNET_TELOPT_BINARY,0)||!clarice_telnet_enabled(engine,TELNET_TELOPT_BINARY,1))return -1;
#endif
  binary_input=cli_alloc(QUEUE);if(!binary_input)return -1;binary_at=binary_count=0;text("Kermit ready; start the peer transfer now.\n");return 0;}
 if(op==4){if(binary_input)cli_free(binary_input);binary_input=0;binary_at=binary_count=0;if(size)closing=1;return 0;}
 if(!binary_input||failed||interrupted)return -1;
 if(op==2){pump();unsigned n=binary_count;if(size<n)n=size;for(unsigned i=0;i<n;i++)((unsigned char*)data)[i]=binary_input[(binary_at+i)%QUEUE];binary_at=(binary_at+n)%QUEUE;binary_count-=n;return n;}
 if(op==3){unsigned long n=0;while(n<size&&!failed&&!interrupted){while(used>QUEUE-512&&!failed&&!interrupted){pump();if(app_yield())interrupted=1;}unsigned k=size-n;if(k>128)k=128;telnet_send(engine,(char*)data+n,k);n+=k;pump();if(app_yield())interrupted=1;}return !failed&&!interrupted?(long)n:-1;}
 return -1;
}
static int stop(void){if(running)return -1;if(release_stream())return -1;
 if(session){cli_session_close(session);session=0;}if(engine){telnet_free(engine);engine=0;}
 session_wire(4,0,0);received_at=received_count=prompt_pending=0;if(driver){DisposePtr((Ptr)driver);driver=0;}wipe(password,sizeof password);wipe(verifier,sizeof verifier);return 0;
}
static int listener_status(void){
#ifndef REMOTE_SERIAL
 if(opening_pending&&!connected&&driver->opening.ioResult<0)return driver->opening.ioResult;
#endif
 return failed?(listener_error?listener_error:2):connected?1:0;
}
static void poll_service(int execute){
 pump();
 if(failed&&!running){unsigned long now=TickCount();if(retry_tick&&(long)(now-retry_tick)<0)return;retry_tick=now+60;int e=release_stream();if(e){listener_error=e;return;}if(session){cli_session_close(session);session=0;}if(engine){telnet_free(engine);engine=0;}
  received_at=received_count=prompt_pending=used=line_used=line_cursor=ready_used=ready_at=bad_line=after_cr=escape=interrupted=eof_pending=closing=0;previous[0]=0;wipe(password,sizeof password);auth_pending=auth_length=auth_overflow=0;failed=0;listener_error=listen_again();if(listener_error)failed=1;else retry_tick=0;return;}
 if(execute&&auth_pending&&!session&&!failed){
  unsigned char result[32];int e=derive(password,result);unsigned difference=0;for(int i=0;i<32;i++)difference|=result[i]^verifier[i];wipe(result,sizeof result);wipe(password,sizeof password);
  auth_pending=0;if(e||difference||auth_overflow||!auth_length){text("\nAuthentication failed.\n");auth_retry=TickCount()+300;closing=1;return;}
  auth_length=0;ClariceSessionIO io={0,read_session,write_session,cancel_session};session=cli_session_open(&io);if(session)cli_session_set_wire(session,session_wire);if(!session){failed=1;return;}cli_session_set_wire(session,session_wire);text("\n");prompt();
 }
 if(execute&&session&&ready_used&&!running&&!failed&&!closing){if(ready_at){memmove(ready,ready+ready_at,ready_used-ready_at);ready_used-=ready_at;ready_at=0;}char command[LINE];unsigned n=0;while(n<ready_used&&ready[n]!='\n')n++;if(n>=LINE){failed=1;return;}memcpy(command,ready,n);command[n]=0;
  strcpy(previous,command);unsigned consumed=n<ready_used?n+1:n;memmove(ready,ready+consumed,ready_used-consumed);ready_used-=consumed;ready_at=0;running=1;interrupted=0;int rc=cli_session_run(session,command);running=0;
  if(rc&256)closing=1;else if(!failed&&!closing){interrupted=0;prompt();}pump();
 }
}
int clarice_main(int argc,char **argv){int explicit_no_password=0;port=SERVICE_SLOT?57600:2323;
#ifdef REMOTE_SERIAL
 if(argc==2&&!strcmp(argv[1],"--help")){const char *h="seriald [--preferences]\nOptional password-protected serial shell. Configure modem/printer in\nPreferences, 57600 baud, 8N1, no flow control. Press Return to log in.\nOne shared remote shell; exit logs out. Three-wire cable removal is not\nreliably detectable. remote off closes both remote transports.\n";cli_write(1,h,strlen(h));return 0;}
 if(argc!=1&&(argc!=2||strcmp(argv[1],"--preferences")))return 2;
#endif
 if(argc==2&&!strcmp(argv[1],"--preferences")){int rc=configure();if(rc!=3)return rc;goto start;}
 if(argc==1){ClariceRemoteSettings settings;memset(&settings,0,sizeof settings);if(config_load(&settings)){const char *m="telnetd: missing/invalid password settings; use Preferences > Telnet server.\n";cli_write(2,m,strlen(m));return 1;}peer=NetParseIP(settings.peer);port=settings.port;protected_login=1;goto start;}
 for(int i=1;i<argc;i++){
  if(!strcmp(argv[i],"--no-password"))explicit_no_password=1;
  else if(!strcmp(argv[i],"--peer")&&i+1<argc)peer=NetParseIP(argv[++i]);
  else if(!strcmp(argv[i],"--port")&&i+1<argc){char *end;unsigned long n=strtoul(argv[++i],&end,10);if(*end||!n||n>65535)return 2;port=n;}
  else{const char *help="telnetd [--preferences] | --no-password --peer IPv4 [--port N]\nOne supplementary plain-text shell; trusted peer required.\nBare telnetd requires saved password settings. remote off stops it.\nOther peers may complete TCP, but are closed before negotiation or login.\nPassword settings: Preferences > Telnet server, or telnetd --preferences.\n1023-byte lines, 4 KiB pending input, arrows/Home/End, last-command Up/Down; overflow rejected.\nTab is whitespace, not completion. Immediate reconnect may require retry.\nSettings: /etc/clarice/telnet.conf (salted verifier, never plaintext).\nPasswords: 1-127 ASCII bytes; 60-second login timeout, 5-second failure cooldown.\nGUI-only commands/editors remain local. Status: 0 success, 1 failure, 2 usage, 130 cancelled.\n";cli_write(1,help,strlen(help));return !strcmp(argv[i],"--help")?0:2;}
 }
 if(!peer||!explicit_no_password){const char *m="telnetd: require --no-password and --peer IPv4 (trusted local network only)\n";cli_write(2,m,strlen(m));return 2;}
 start:;
 OSErr start_error=NetInit();if(start_error){char message[128];snprintf(message,sizeof message,"telnetd: %s (Mac OS %d)\n",NetErrStr(start_error),start_error);cli_write(2,message,strlen(message));return 1;}driver=(DriverStorage*)NewPtrSysClear(sizeof *driver);if(!driver)return 1;
#ifdef REMOTE_SERIAL
 if(listen_again()){if(!release_stream()){DisposePtr((Ptr)driver);driver=0;}return 1;}
 ClariceService service={poll_service,stop};if(cli_service_register_status_slot(SERVICE_SLOT,&service,listener_status)){stop();return 1;}
#else
 if(listen_again())goto startup_failed;
 ClariceService service={poll_service,stop};if(cli_service_register_status_slot(SERVICE_SLOT,&service,listener_status))goto startup_failed;
#endif
 const char *m=SERVICE_NAME " enabled for the configured endpoint. Unencrypted; use remote off to stop.\n";cli_write(1,m,strlen(m));return 0;
#ifndef REMOTE_SERIAL
 startup_failed:
 /* Before registration no resident service owns this image. Do not return
  * through the loader while MacTCP still references DriverStorage. */
 while(stop())net_cleanup_pause();
 return 1;
#endif
}
