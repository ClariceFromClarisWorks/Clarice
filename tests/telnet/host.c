#define TELNET_HOST_TEST
#include "../../src/commands/telnet.c"
#include <assert.h>
static int native_cancel;
static unsigned char sent[16384],shown[16384],input[4096];
static unsigned sn,on,in,off,ticks,polls;
static int allocations,allocfail,screenfail,fail_at,alloc_calls;
static size_t live_bytes,peak_bytes;
static int status_error;
static int active,terminal,connectfail,sendfail,outfail,cancel,overflow,escape_test,closed,ttys;
int cli_isatty(int f){(void)f;return ttys;}
long cli_write(int f,const void *p,unsigned long n){if(f==2)return n;if(outfail)return -1;assert(on+n<sizeof shown);memcpy(shown+on,p,n);on+=n;return n;}
int app_serial_tty(int x){terminal=x;return 0;}void app_activity_interactive(int x){active=x;}
int app_serial_key(void){if(overflow)return -2;if(escape_test){int n=escape_test++;if(n==1)return 29;if(n==2)return 'q';}return -1;}
unsigned long cli_ticks(void){return ++ticks;}
int app_yield(void){polls++;return cancel&&polls>2;}
int NetResolve(const char *s,unsigned long *ip,int(*p)(void),unsigned long t){(void)s;(void)p;(void)t;*ip=1;return 0;}
int NetConnect(NetConn *c,unsigned long ip,unsigned short p){if(native_cancel==1)return userCanceledErr;assert(ip==1&&p==23);if(connectfail)return -108;c->open=1;return 0;}
int NetClose(NetConn *c){c->open=0;return native_cancel==3?userCanceledErr:0;}
int NetSend(NetConn *c,const void *p,unsigned short n){if(native_cancel==2)return userCanceledErr;(void)c;if(sendfail)return -1;assert(sn+n<sizeof sent);memcpy(sent+sn,p,n);sn+=n;return 0;}
long NetAvailable(NetConn *c){(void)c;if(status_error)return status_error;return off<in?1:closed?connectionClosing:0;}
long NetRecv(NetConn *c,void *p,unsigned short n,unsigned char t){(void)c;(void)n;(void)t;*(unsigned char*)p=input[off++];return 1;}
static void reset(void){status_error=0;assert(!active&&!terminal);if(engine){telnet_free(engine);engine=0;}protocol_failed=0;dn=tn=cr=escaped=mac_error=0;sn=on=in=off=ticks=polls=0;connectfail=sendfail=outfail=cancel=overflow=escape_test=closed=allocfail=screenfail=0;ttys=1;dumb=1;reason="test";assert(protocol_open());}
static void feed(const unsigned char *p,unsigned n){for(unsigned i=0;i<n;i++)assert(receive(p[i]));assert(flush_tx()&&flush_display());}
static int run(void){char *a[]={"telnet","--dumb","host"};telnet_free(engine);engine=0;int r=clarice_main(3,a);assert(!active&&!terminal&&!conn.open);return r;}
int app_columns(void){return 80;}int app_screen_rows(void){return 24;}
int app_screen_pending(void){return overflow;}int app_screen_open(void){if(screenfail)return -1;terminal=2;return 0;}
void app_screen_close(void){terminal=0;}long cli_read(int f,void*p,unsigned long n){(void)f;(void)p;(void)n;return -1;}
void *cli_alloc(unsigned long n){size_t *p;if(allocfail||(++alloc_calls==fail_at))return 0;p=calloc(1,n+sizeof *p);assert(p);*p=n;allocations++;live_bytes+=n;if(live_bytes>peak_bytes)peak_bytes=live_bytes;return p+1;}
void cli_free(void*p){if(p){size_t *h=(size_t*)p-1;live_bytes-=*h;allocations--;free(h);}}
int main(void){
 for(int phase=1;phase<=3;phase++){reset();native_cancel=phase;closed=1;input[in++]=IAC;input[in++]=DO;input[in++]=TTYPE;assert(run()==130&&!allocations&&!live_bytes&&!conn.open&&!terminal);native_cancel=0;}

 reset();unsigned char a[]={IAC,WILL,ECHO,IAC,WILL,ECHO,IAC,DO,TTYPE,IAC,SB,TTYPE,1,IAC,SE};feed(a,sizeof a);
 unsigned char expected[]={IAC,DO,ECHO,IAC,WILL,TTYPE,IAC,SB,TTYPE,0,'D','U','M','B',IAC,SE};assert(sn==sizeof expected&&!memcmp(sent,expected,sn)&&!on);
 assert(send_key('a')&&flush_tx()&&flush_display()&&!on);
 unsigned char offecho[]={IAC,WONT,ECHO,IAC,WONT,ECHO,IAC,DO,ECHO,IAC,WILL,0};sn=0;feed(offecho,sizeof offecho);unsigned char no[]={IAC,DONT,ECHO,IAC,WONT,ECHO,IAC,DONT,0};assert(sn==sizeof no&&!memcmp(sent,no,sn));
 reset();unsigned char text[]={'a','\r',0,'b','\r','\n',IAC,IAC};feed(text,sizeof text);unsigned char want[]={'a','\r','b','\r','\n',255};assert(on==sizeof want&&!memcmp(shown,want,on));
 reset();assert(send_key(13)&&send_key(8)&&send_key(255)&&send_key(3)&&flush_tx());unsigned char keys[]={13,10,127,255,255,255,244};assert(sn==sizeof keys&&!memcmp(sent,keys,sn));
 reset();assert(receive(IAC)&&receive(SB));for(int i=0;i<16385;i++)assert(receive(42));assert(!receive(42));
 reset();assert(receive(IAC)&&receive(SB)&&receive(42)&&receive(IAC)&&!receive(42));
 reset();input[in++]='x';closed=1;assert(run()==0&&on==1);
 reset();input[in++]=IAC;closed=1;assert(run()==1);
 reset();input[in++]=IAC;assert(run()==1&&ticks>=1800);
 reset();cancel=1;assert(run()==130);
 reset();overflow=1;assert(run()==1);
 reset();escape_test=1;assert(run()==0);
 reset();status_error=-23012;assert(run()==1&&mac_error==-23012);
 reset();connectfail=1;assert(run()==1);
 reset();input[in++]=IAC;input[in++]=WILL;input[in++]=ECHO;sendfail=1;assert(run()==1);
 reset();input[in++]='x';outfail=1;assert(run()==1);
 reset();ttys=0;assert(run()==2);
 reset();telnet_free(engine);engine=0;char *help[]={"telnet","--help"};assert(clarice_main(2,help)==0);char *bad[]={"telnet","host","65536"};assert(clarice_main(3,bad)==2);
 for(int i=0;i<20;i++){reset();closed=1;assert(run()==0);}
 reset();telnet_free(engine);engine=0;char *va[]={"telnet","host"};allocfail=1;assert(clarice_main(2,va)==1&&!allocations&&!terminal&&!conn.open);
 reset();telnet_free(engine);engine=0;screenfail=1;assert(clarice_main(2,va)==1&&!allocations&&!terminal&&!conn.open);
 reset();telnet_free(engine);engine=0;closed=1;assert(clarice_main(2,va)==0&&!allocations&&!terminal&&!conn.open);
 reset();telnet_free(engine);engine=0;cancel=1;assert(clarice_main(2,va)==130&&!allocations&&!terminal&&!conn.open);
 reset();telnet_free(engine);engine=0;overflow=1;assert(clarice_main(2,va)==1&&!allocations&&!terminal&&!conn.open);
 reset();telnet_free(engine);engine=0;closed=1;outfail=1;assert(clarice_main(2,va)==1&&!allocations&&!terminal&&!conn.open);
 for(int i=0;i<20;i++){reset();telnet_free(engine);engine=0;closed=1;assert(clarice_main(2,va)==0&&!allocations&&!terminal&&!conn.open);}
 reset();telnet_free(engine);engine=0;input[in++]=27;input[in++]='[';closed=1;assert(clarice_main(2,va)==1&&!allocations&&!terminal&&!conn.open);
 reset();vt=cli_alloc(sizeof *vt);memset(vt,0,sizeof *vt);vt->reply=reply;vt_reset(vt,4,8);dumb=0;
 unsigned char opts[]={IAC,DO,TTYPE,IAC,SB,TTYPE,1,IAC,SE,IAC,DO,NAWS};feed(opts,sizeof opts);
 unsigned char vtoptions[]={IAC,WILL,TTYPE,IAC,SB,TTYPE,0,'V','T','1','0','0',IAC,SE,IAC,WILL,NAWS,IAC,SB,NAWS,0,8,0,4,IAC,SE};assert(sn==sizeof vtoptions&&!memcmp(sent,vtoptions,sn));
 sn=0;assert(show(27)&&show('[')&&show('?')&&show('1')&&show('h'));assert(terminal_key(27)&&terminal_key('[')&&terminal_key('A')&&flush_tx());assert(sn==3&&!memcmp(sent,"\033OA",3));
 /* Present the model through the unchanged Core parser. Dirty contents, all
    cells/attributes and cursor must agree; no bitmap/rendering fork. */
 CliScreen core;screen_init(&core,4,8);memset(core.cells,'z',sizeof core.cells);
 on=dn=0;memset(vt->s.dirty,1,sizeof vt->s.dirty);
 const char *sample="abcdefghZ\033[2;3H\033[7mREV\033[0m\033[4;8HX";
 while(*sample)assert(show(*sample++));assert(flush_display());screen_feed(&core,shown,on);
 for(int r=0;r<4;r++)for(int c=0;c<8;c++){assert(core.cells[r][c]==(vt->s.cells[r][c]?vt->s.cells[r][c]:' '));assert(core.attrs[r][c]==vt->s.attrs[r][c]);}
 assert(core.row==vt->s.row&&core.col==vt->s.col);cli_free(vt);vt=0;telnet_free(engine);engine=0;assert(!allocations);
 /* Fail each protocol/screen allocation in a realistic negotiation, then
    require a subsequent successful invocation with no retained capacity. */
 size_t ordinary_peak=0;
 for(int fail=1;fail<=8;fail++){
  reset();telnet_free(engine);engine=0;assert(!live_bytes&&!allocations);
  unsigned char session[]={IAC,WILL,ECHO,IAC,DO,SGA,IAC,DO,NAWS,IAC,DO,TTYPE,IAC,SB,TTYPE,1,IAC,SE,'o','k'};
  memcpy(input,session,sizeof session);in=sizeof session;closed=1;
  alloc_calls=0;fail_at=fail;peak_bytes=0;
  int r=clarice_main(2,va);assert(r==(alloc_calls>=fail?1:0));
  assert(!allocations&&!live_bytes&&!terminal&&!conn.open);
  if(r==0)ordinary_peak=peak_bytes;
  fail_at=0;reset();closed=1;assert(run()==0&&!allocations&&!live_bytes);
 }
 /* Exhaust each upstream SB growth allocation while retaining the old
    buffer; fatal callback must leave it available to telnet_free. */
 for(int growth=1;growth<=4;growth++){
  reset();alloc_calls=0;fail_at=growth;int ok=1;
  assert(receive(IAC)&&receive(SB)&&receive(42));
  for(int n=0;n<17000&&ok;n++)ok=receive('x');
  assert(!ok&&protocol_failed);fail_at=0;telnet_free(engine);engine=0;
  assert(!allocations&&!live_bytes);
 }
 /* General Q-method crossing/queued reversal, exercised without adding a
    new public client option: upstream owns both directions and pending state. */
 reset();telnet_negotiate(engine,DO,ECHO);telnet_negotiate(engine,DONT,ECHO);
 assert(receive(IAC)&&receive(WILL)&&receive(ECHO));
 assert(receive(IAC)&&receive(WONT)&&receive(ECHO));assert(flush_tx());
 unsigned char queued[]={IAC,DO,ECHO,IAC,DONT,ECHO};assert(sn==sizeof queued&&!memcmp(sent,queued,sn));
 assert(!clarice_telnet_enabled(engine,ECHO,0));
 telnet_free(engine);engine=0;assert(!allocations&&!live_bytes);
 printf("PASS ownership: injected allocation failures, subsequent recovery, Q queued reversal; ordinary host peak %zu bytes\n",ordinary_peak);
 puts("PASS Telnet: fragmented NVT/IAC, echo transitions/duplicates, TTYPE, refused options, CRLF/CRNUL, escaped IAC, IP, malformed/bounded controls, timeout, EOF, cancel/escape, overflow, connect/send/output failures and repeated restoration");
}
