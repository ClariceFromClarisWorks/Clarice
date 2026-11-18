#include "../../src/commands/remote/telnetd.c"
#include <assert.h>
static int cancel_auth;static unsigned allocations,execs;static int fail_memory;static int alive,abort_delay,hang_send;static TCPiopb *pending_open,*pending_send;
static char incoming[2048],wire[16384];static unsigned in_count,wire_count;static unsigned long ticks=1;
Ptr NewPtrSys(unsigned long n){if(fail_memory)return NULL;Ptr p=malloc(n);if(p)allocations++;return p;}
Ptr NewPtrSysClear(unsigned long n){Ptr p=NewPtrSys(n);if(p)memset(p,0,n);return p;}
void DisposePtr(Ptr p){assert(p);assert(p!=(Ptr)driver||!alive);allocations--;free(p);}
void *cli_alloc(unsigned long n){return NewPtrSys(n);}void cli_free(void *p){DisposePtr(p);}
short MemError(void){return -108;}unsigned long TickCount(void){return ticks++;}
short OpenDriver(ConstStringPtr p,short *r){(void)p;*r=-1;return 0;}
short PBControlSync(ParmBlkPtr p0){TCPiopb *p=p0;switch(p->csCode){
case TCPCreate:assert(!alive);alive=1;p->tcpStream=(Ptr)1;break;
case TCPAbort:if(abort_delay)return 0;if(pending_open){pending_open->ioResult=-1;pending_open=0;}if(pending_send){pending_send->ioResult=-1;pending_send=0;}break;
case TCPRelease:assert(!pending_open&&!pending_send);alive=0;break;
case TCPStatus:p->csParam.status.amtUnreadData=in_count;p->csParam.status.connectionState=8;break;
case TCPRcv:{unsigned n=p->csParam.receive.rcvBuffLen;if(n>in_count)n=in_count;memcpy(p->csParam.receive.rcvBuff,incoming,n);memmove(incoming,incoming+n,in_count-n);in_count-=n;p->csParam.receive.rcvBuffLen=n;break;}
default:break;}return 0;}
short PBControlAsync(ParmBlkPtr p0){TCPiopb *p=p0;p->ioResult=1;if(p->csCode==TCPPassiveOpen)pending_open=p;else if(p->csCode==TCPClose){p->ioResult=0;}else if(p->csCode==TCPSend){assert(!pending_send);pending_send=p;if(!hang_send){memcpy(wire+wire_count,driver->send,driver->wds.length);wire_count+=driver->wds.length;p->ioResult=0;pending_send=0;}}return 0;}
long cli_write(int fd,const void *p,unsigned long n){(void)fd;(void)p;return n;}
int cli_service_register_slot(int slot,const ClariceService *p){assert(slot==0);assert(p->poll==poll_service&&p->stop==stop);return 0;}
void *cli_session_open(const ClariceSessionIO *p){assert(p->read&&p->write&&p->cancelled);return (void*)1;}
int cli_session_close(void *p){assert(p==(void*)1);return 0;}
void cli_session_prompt(void *p,char *out,unsigned long cap){(void)p;snprintf(out,cap,"/ > ");}
int cli_session_run(void *p,const char *cmd){assert(p==(void*)1);assert(!strcmp(cmd,"echo good"));execs++;return 0;}
int app_yield(void){pump();return cancel_auth;}
static void feed_bytes(const char *p,unsigned n){in_count=n;assert(in_count<sizeof incoming);memcpy(incoming,p,in_count);while(in_count)pump();}
static void feed(const char *p){feed_bytes(p,(unsigned)strlen(p));}
/* Configuration native boundary: independently exercised in Snow. */
static char config_fixture[256];
int cli_file_open(const char *p,const char *m){(void)p;(void)m;return config_fixture[0]?1:-1;}
long cli_file_read(int f,void *p,unsigned long n){(void)f;size_t size=strlen(config_fixture);if(size>n)size=n;memcpy(p,config_fixture,size);return size;}
long cli_file_write(int f,const void *p,unsigned long n){(void)f;(void)p;return n;}
int cli_file_close(int f){(void)f;return 0;}
void *cli_replace_begin(const char *p,char *q,unsigned long n){(void)p;(void)q;(void)n;return 0;}
int cli_replace_commit(void *p){(void)p;return -1;}
void cli_replace_abort(void *p){(void)p;}
unsigned long cli_mac_time(void){return 1;}
int cli_service_stop_slot(int slot){assert(slot==0);return stop();}
int cli_service_enabled_slot(int slot){assert(slot==0);return driver!=0;}
int app_remote_settings(ClariceRemoteSettings *p){(void)p;return 0;}
void app_remote_indicator(int slot,int on){(void)slot;(void)on;}
int app_serial_remote_settings(ClariceRemoteSettings *p){(void)p;return 0;}
int cli_session_set_wire(void *s,ClariceSessionWire w){assert(s==(void*)1&&w==session_wire);return 0;}
int main(void){char *av[]={"telnetd","--no-password","--peer","10.0.0.1",0};
 fail_memory=1;assert(clarice_main(4,av)==1&&!allocations&&!alive);fail_memory=0;
 assert(!clarice_main(4,av));assert(alive&&pending_open);pending_open->ioResult=0;pending_open=0;poll_service(0);assert(session&&engine);
 feed("\377\375\001");pump();unsigned before=wire_count;feed("x");pump();assert(wire_count>before);feed("\025");pump();
 feed("\377\376\001");pump();before=wire_count;feed("x");pump();assert(wire_count==before);feed("\025");
 feed("echo good\r\n");assert(!execs);poll_service(1);assert(execs==1);
 char longline[1500];memset(longline,'x',sizeof longline-1);longline[sizeof longline-1]=0;feed(longline);feed("\r");poll_service(1);assert(execs==1&&!ready_used);
 feed("echo good\r\n");poll_service(1);assert(execs==2);
 feed("echo good\r\necho good\r\n");poll_service(1);poll_service(1);assert(execs==4);
 /* Eight-bit negotiation must not turn ordinary shell LF into stair-step text. */
 feed_bytes("\377\375\000",3);pump();assert(clarice_telnet_enabled(engine,TELNET_TELOPT_BINARY,1));
 before=used;terminal_text("a\nb",3);assert(used==before+4&&!memcmp(output+before,"a\r\nb",4));
 before=used;telnet_send(engine,"a\nb",3);assert(used==before+3&&!memcmp(output+before,"a\nb",3));
 /* Logout must drain both the queue and pending native write before close. */
 text("LOGOUT_LAST");closing=1;pump();pump();pump();assert(failed);poll_service(0);assert(pending_open);pending_open->ioResult=0;pending_open=0;poll_service(0);
 feed("bad\003");assert(!line_used&&!ready_used&&!interrupted);
 hang_send=1;text("pending");pump();assert(pending_send);abort_delay=1;assert(stop());assert(driver&&alive&&pending_send);abort_delay=0;assert(!stop());assert(!allocations&&!alive&&!pending_send);
 /* Reconnection resets both command state and native buffer ownership. */
 failed=used=line_used=ready_used=ready_at=0;hang_send=0;assert(!clarice_main(4,av));pending_open->ioResult=0;pending_open=0;poll_service(0);failed=1;poll_service(0);assert(pending_open&&!session&&!engine);assert(!stop());assert(!allocations&&!alive);

 /* PBKDF2 known answer, no shell before auth, no echoed secret, pipelined
    pre-auth command discarded, rejection cooldown, cancellation and cleanup. */
 unsigned char result[32];memset(salt,0,sizeof salt);assert(!derive("password",result));
 char encoded[65];hex(encoded,result,32);
 assert(!strcmp(encoded,"7460518eb1741d7be7b2914828b97011abdb01f2e6e94ff954490a5c74be5554"));
 cancel_auth=1;assert(derive("password",result)==-1);cancel_auth=0;assert(!derive("password",result));
 memcpy(verifier,result,32);protected_login=1;failed=closing=used=auth_retry=0;
 assert(!clarice_main(4,av));pending_open->ioResult=0;pending_open=0;poll_service(0);assert(!session);
 feed("wrong\r\necho good\r\n");assert(auth_pending&&!session&&!ready_used);poll_service(1);assert(closing&&!session);assert(!stop());
 failed=closing=used=0;auth_pending=auth_length=auth_overflow=0;auth_retry=0;memcpy(verifier,result,32);
 assert(!clarice_main(4,av));pending_open->ioResult=0;pending_open=0;poll_service(0);assert(!session);
 feed("password\r\necho good\r\n");unsigned old_execs=execs;poll_service(1);assert(session&&!ready_used&&execs==old_execs);assert(!password[0]);
 feed("echo good\r\n");poll_service(1);assert(execs==old_execs+1);assert(!stop());assert(!allocations);

 ClariceRemoteSettings settings;char sh[33],vh[65];hex(sh,salt,16);hex(vh,result,32);
 snprintf(config_fixture,sizeof config_fixture,"CLARICE-TELNET-1 10.0.0.1 2323 1000 %s %s\n",sh,vh);assert(!config_load(&settings));assert(settings.port==2323&&!strcmp(settings.peer,"10.0.0.1"));
 strcat(config_fixture,"extra");assert(config_load(&settings));strcpy(config_fixture,"CLARICE-TELNET-1 10.0.0.1 2323 1000 broken");assert(config_load(&settings));config_fixture[0]=0;assert(config_load(&settings));
 puts("PASS authentication: upstream PBKDF2 known answer; no pre-auth shell or pipelined execution; wrong/right password, wiped input, strict missing/malformed config; zero retained allocations");
 puts("PASS service: allocation failure, graceful logout drain, negotiated echo, idle dispatch, CRLF, overlong input rejection/recovery, interrupt, delayed abort retains buffers, reconnect and zero retained allocations");
}
