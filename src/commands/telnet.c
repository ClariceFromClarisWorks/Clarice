/* Native Macintosh Telnet client using genuine upstream libtelnet.
 * Transport and rendering remain Clarice services; protocol state is upstream. */
#ifdef TELNET_HOST_TEST
#include "../../tests/telnet/shim.h"
#else
#include "../sys6.h"
#include "network/native.h"
#include <Errors.h>
#include <stdio.h>
#include <string.h>
#endif
extern int cli_isatty(int),app_serial_tty(int),app_serial_key(void);
extern long cli_write(int,const void *,unsigned long);
extern unsigned long cli_ticks(void);
#define IAC 255
#define WILL 251
#define WONT 252
#define DO 253
#define DONT 254
#define SB 250
#define SE 240
#define ECHO 1
#define SGA 3
#define TTYPE 24
#define NAWS 31
#include "terminal/vt100.h"
#include "network/telnet-engine.h"
#include "../help-output.h"
extern void *cli_alloc(unsigned long);
extern void cli_free(void *);
extern long cli_read(int,void *,unsigned long);
extern int app_screen_open(void),app_screen_rows(void),app_screen_pending(void),app_columns(void);
extern void app_screen_close(void);
static Vt100 *vt;
static int dumb,lastrow,lastcol,lastcursor,key_state;
static unsigned long key_tick,paint_tick;
static unsigned paint_bytes;
static NetConn conn;
static unsigned char rx[512],tx[256],display[512];
static unsigned dn,tn;
static telnet_t *engine;
static int protocol_failed;
static int cr,escaped,mac_error;
static unsigned long protocol_tick;
static const char *reason;
static int raw_flush(void){if(dn&&cli_write(1,display,dn)!=(long)dn){reason="terminal output failed";return 0;}dn=0;return 1;}
static int raw_show(unsigned char c){if(dn==sizeof display&&!raw_flush())return 0;display[dn++]=c;return 1;}
static int show(unsigned char c){if(vt){if(!vt_byte(vt,c)){reason="oversized or invalid terminal control";return 0;}return 1;}return raw_show(c);}
static int paint_text(const char *s){while(*s)if(!raw_show(*s++))return 0;return 1;}
static int flush_display(void){
 if(vt){CliScreen *s=&vt->s;int changed=lastrow!=s->row||lastcol!=s->col||lastcursor!=s->cursor;
  for(int r=0;r<s->rows;r++)if(s->dirty[r]){char pos[32];int attr=-1;changed=1;
   snprintf(pos,sizeof pos,"\033[%d;1H",r+1);if(!paint_text(pos))return 0;
   for(int c=0;c<s->cols;c++){int a=s->attrs[r][c]^vt->global_reverse;if(a!=attr){if(!paint_text(a?"\033[7m":"\033[0m"))return 0;attr=a;}if(!raw_show(s->cells[r][c]?s->cells[r][c]:' '))return 0;}
   s->dirty[r]=0;
  }
  if(changed){char pos[48];snprintf(pos,sizeof pos,"\033[0m\033[%d;%dH\033[?25%c",s->row+1,s->col+1,s->cursor?'h':'l');if(!paint_text(pos))return 0;}
  lastrow=s->row;lastcol=s->col;lastcursor=s->cursor;
 }
 paint_tick=cli_ticks();paint_bytes=0;return raw_flush();
}
static int flush_tx(void){if(tn){mac_error=NetSend(&conn,tx,tn);if(mac_error){reason="send failed";return 0;}}tn=0;return 1;}
static int wire(unsigned char c){if(tn==sizeof tx&&!flush_tx())return 0;tx[tn++]=c;return 1;}
static int reply(void *context,const char *s,unsigned n){(void)context;telnet_send(engine,s,n);return !protocol_failed;}
static int size_reply(void){unsigned char bytes[4];int cols=vt?vt->s.cols:app_columns(),rows=vt?vt->s.rows:app_screen_rows();
 bytes[0]=cols>>8;bytes[1]=cols;bytes[2]=rows>>8;bytes[3]=rows;
 telnet_subnegotiation(engine,NAWS,(const char *)bytes,sizeof bytes);return !protocol_failed;}
static void protocol_event(telnet_t *t,telnet_event_t *e,void *context){
 (void)context;if(protocol_failed)return;
 switch(e->type){
 case TELNET_EV_SEND:
  for(size_t i=0;i<e->data.size;i++)if(!wire((unsigned char)e->data.buffer[i])){protocol_failed=1;break;}break;
 case TELNET_EV_DATA:
  for(size_t i=0;i<e->data.size;i++){unsigned char c=e->data.buffer[i];
   /* Keep CR LF for the native screen; only remove NVT CR padding. */
   if(cr){cr=0;if(!c)continue;}if(c=='\r')cr=1;
   if(!show(c)){protocol_failed=1;break;}}
  break;
 case TELNET_EV_DO:if(e->neg.telopt==NAWS&&!size_reply())protocol_failed=1;break;
 case TELNET_EV_TTYPE:
  if(e->ttype.cmd==TELNET_TTYPE_SEND&&clarice_telnet_enabled(t,TTYPE,1))
   telnet_ttype_is(t,dumb?"DUMB":"VT100");break;
 case TELNET_EV_IAC:if(e->iac.cmd==TELNET_EC&&!show(8))protocol_failed=1;break;
 case TELNET_EV_ERROR:case TELNET_EV_WARNING:
  /* Upstream messages are callback-local: never retain their pointers. */
  reason=e->error.errcode==TELNET_ENOMEM?"insufficient memory for Telnet protocol":"invalid or oversized Telnet control";
  protocol_failed=1;break;
 default:break;
 }
}
static int protocol_open(void){
 static const telnet_telopt_t options[]={
  {ECHO,TELNET_WONT,TELNET_DO},{SGA,TELNET_WILL,TELNET_DO},
  {TTYPE,TELNET_WILL,TELNET_DONT},{NAWS,TELNET_WILL,TELNET_DONT},
  {-1,0,0}};
 engine=telnet_init(options,protocol_event,0,0);
 if(!engine){reason="insufficient memory for Telnet protocol";return 0;}return 1;
}
static int receive(unsigned char c){
 int pending=clarice_telnet_pending(engine);
 telnet_recv(engine,(const char *)&c,1);
 if(!pending&&clarice_telnet_pending(engine))protocol_tick=cli_ticks();
 return !protocol_failed;
}
/* ETX maps to Telnet IP, without TCP urgent/SYNCH. Return is NVT CR LF. */
static int send_key(unsigned c){
 if(c==3){telnet_iac(engine,TELNET_IP);return !protocol_failed;}
 if(c==8)c=127;
 {char ch=c;telnet_send(engine,&ch,1);if(c=='\r')telnet_send(engine,"\n",1);if(protocol_failed)return 0;}
 if(!clarice_telnet_enabled(engine,ECHO,0)){
  if(c==127)return show(8)&&show(' ')&&show(8);
  if(c=='\r')return show('\r')&&show('\n');
  if(c>=32||c=='\t')return show(c);
 }return 1;
}
/* Core screen keys are normal CSI arrows; honour remote DECCKM without
 * confusing a physical arrow with the explicit Ctrl+] local escape. */
static int terminal_key(unsigned c){
 if(!vt)return send_key(c);
 if(key_state==1){if(c=='['){key_state=2;return 1;}key_state=0;if(!send_key(27))return 0;}
 else if(key_state==2){key_state=0;if(c>='A'&&c<='D'){return wire(27)&&wire(vt->appcursor?'O':'[')&&wire(c);}if(!send_key(27)||!send_key('['))return 0;}
 if(c==27){key_state=1;key_tick=cli_ticks();return 1;}return send_key(c);
}
static void message(const char *s){cli_write(2,s,strlen(s));}
static const char usage[]="usage: telnet [--dumb] HOST [PORT]\n";
int clarice_main(int argc,char **argv){
 unsigned long ip=0,port=23;int result=1,tty=0;memset(&conn,0,sizeof conn);engine=0;protocol_failed=0;
 dn=tn=0;cr=escaped=mac_error=0;vt=0;dumb=key_state=0;lastrow=lastcol=lastcursor=-1;paint_tick=0;paint_bytes=0;reason="connection failed";
 if(argc==2&&!strcmp(argv[1],"--help"))return help_output(1,
  "Usage:\n"
  "  telnet [--dumb] HOST [PORT]\n"
  "Use a monochrome VT100/ANSI terminal over unencrypted TCP. Default port: 23. Both streams must be terminals.\n"
  "Options:\n"
  "  --dumb\tUse the line-oriented NVT instead of a full-screen terminal.\n"
  "Ctrl+] then q exits; Ctrl+] twice sends Ctrl+]. Command-period cancels locally. Ctrl+C sends Telnet IP.\n"
  "Return sends CR LF; Backspace sends DEL. Application arrows are supported.\n"
  "Uses libtelnet and TERM=vt100 with NAWS. Resize before connecting; size is fixed for the session.\n"
  "Supports inverse, cursor movement, erase, scroll regions, insert/delete and ASCII line graphics.\n"
  "No colour, UTF-8, double-size rows, alternate-screen history, bold, underline, blink or VT52.\n"
  "No TLS, autologin, transfers or urgent SYNCH. DNS/TCP may delay cancellation.\n"
  "No idle timeout. Incomplete controls time out after 30 seconds. Overflow ends the session.\n"
  "Status: 0 peer close/local escape, 1 error, 2 usage, 130 cancelled.\n");
 if(argc>1&&!strcmp(argv[1],"--dumb")){dumb=1;argc--;argv++;}
 if(argc<2||argc>3||argv[1][0]=='-')goto usage;
 if(argc==3){port=0;if(!*argv[2])goto usage;for(const char *p=argv[2];*p;p++){if(*p<'0'||*p>'9'||port>6553)goto usage;port=port*10+*p-'0';}if(!port||port>65535)goto usage;}
 if(!cli_isatty(0)||!cli_isatty(1)){message("telnet: interactive terminal input/output required\n");return 2;}
 if(!protocol_open())goto end;
 reason="system hostname resolution";mac_error=NetResolve(argv[1],&ip,app_yield,1800);if(mac_error)goto end;
 reason="opening connection";mac_error=NetConnect(&conn,ip,port);if(mac_error)goto end;
 if(dumb){app_serial_tty(1);tty=1;app_activity_interactive(1);}
 else{vt=cli_alloc(sizeof *vt);if(!vt){reason="insufficient memory for VT100 screen";goto end;}memset(vt,0,sizeof *vt);vt->reply=reply;
  if(app_screen_open()){reason="cannot acquire terminal screen";goto end;}tty=2;vt_reset(vt,app_screen_rows(),app_columns());}
 message("Telnet connected. Ctrl+] then q exits; Cmd-period cancels.\n");
 for(;;){
  if(app_yield()){result=130;break;}
  long n=NetAvailable(&conn);
  if(n<0&&n!=connectionClosing){mac_error=n;reason="connection status failed";goto end;}
  if(n==connectionClosing){if(clarice_telnet_pending(engine)){reason="connection ended inside Telnet control";goto end;}if(vt&&vt->parser){reason="connection ended inside terminal control";goto end;}result=0;break;}
  int active=n!=0;
  if(n){if(n>(long)sizeof rx)n=sizeof rx;n=NetRecv(&conn,rx,n,1);if(n<=0){if(n<0)mac_error=n;reason="receive failed";goto end;}
   for(long i=0;i<n;i++)if(!receive(rx[i]))goto end;paint_bytes+=(unsigned)n;
  }
  if(clarice_telnet_pending(engine)&&cli_ticks()-protocol_tick>=1800){reason="incomplete Telnet control timeout";goto end;}
  for(unsigned i=0;i<64;i++){
   int c;if(vt){unsigned char key;if(!app_screen_pending())break;c=cli_read(0,&key,1)==1?key:-2;}else c=app_serial_key();if(c==-1)break;if(c<0){reason="keyboard queue overflow; session stopped";goto end;}
   active=1;
   if(escaped){escaped=0;if(c=='q'){result=0;goto end;}if(c!=29&&!terminal_key(29))goto end;}
   else if(c==29){escaped=1;continue;}
   if(!terminal_key(c))goto end;
  }
  if(key_state&&cli_ticks()-key_tick>=3){int pending=key_state;key_state=0;if(!send_key(27)||(pending==2&&!send_key('[')))goto end;}
  if(!flush_tx())goto end;
  /* Consume burst output before painting; keep a bounded 4 KiB frame budget
     and service keys/cancellation on every 512-byte receive iteration. */
  if(!vt||n<(long)sizeof rx||paint_bytes>=4096)if(!flush_display())goto end;
  /* Do not delay received text or keys. Idle connections sleep one tick
     through the existing event/service wait instead of spinning TCPStatus. */
  if(!active&&cli_pause_ticks(1)){result=130;break;}
 }
end:
 if(tty){if(!flush_display()&&!result)result=1;if(tty==2)app_screen_close();else app_serial_tty(0);app_activity_interactive(0);}if(vt){cli_free(vt);vt=0;}
 if(engine){telnet_free(engine);engine=0;}
 {int e=NetClose(&conn);if(e){message("telnet: network cleanup reported an error (resources released)\n");if(e==userCanceledErr||!mac_error)mac_error=e;if(!result)result=1;}}
 if(mac_error==userCanceledErr)result=130;
 if(result==1){char b[160];snprintf(b,sizeof b,"\ntelnet: %s (Mac error %d)\n",reason,mac_error);message(b);}
 else message(result==130?"\nTelnet cancelled.\n":"\nTelnet connection closed.\n");
 return result;
usage:message(usage);return 2;
}
