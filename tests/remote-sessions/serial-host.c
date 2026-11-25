#include "../../src/commands/remote/seriald.c"
#include <assert.h>
static unsigned allocations,executed;static int fail_alloc,delay_abort,hang_send,overrun;
static unsigned long tick=1;static ParamBlockRec *pending;static char input[1024];static unsigned input_size;static int input_open,output_open,buf_owned;
static int open_fail,reset_fail,register_fail,detach_fail,close_in_fail,close_out_fail;
static unsigned cleanup_pauses;
Ptr NewPtrSys(unsigned long n){if(fail_alloc)return 0;void *p=malloc(n);if(p)allocations++;return p;}
Ptr NewPtrSysClear(unsigned long n){Ptr p=NewPtrSys(n);if(p)memset(p,0,n);return p;}
void DisposePtr(Ptr p){if(p==(Ptr)driver)assert(!input_open&&!output_open&&!buf_owned&&!pending);allocations--;free(p);}
void *cli_alloc(unsigned long n){return NewPtrSys(n);}void cli_free(void *p){DisposePtr(p);}
unsigned long TickCount(void){return tick++;}short MemError(void){return -108;}
short OpenDriver(ConstStringPtr name,short *ref){if(name[0]==4){input_open=1;*ref=1;}else{if(open_fail)return -36;output_open=1;*ref=2;}return 0;}
int CloseDriver(short ref){int *fault=ref==1?&close_in_fail:&close_out_fail;if(*fault){--*fault;return -36;}if(ref==1){assert(input_open&&!buf_owned);input_open=0;}else{assert(output_open&&!pending);output_open=0;}return 0;}
int SerSetBuf(short ref,Ptr p,long size){(void)ref;(void)size;if(!p&&detach_fail){--detach_fail;return -36;}buf_owned=p!=0;return 0;}
int SerReset(short r,int s){(void)r;(void)s;return reset_fail?-36:0;}int SerHShake(short r,SerShk *s){(void)r;assert(!s->fXOn&&!s->fCTS);return 0;}
int SerStatus(short r,SerStaRec *s){(void)r;s->cumErrs=overrun;return 0;}
int SerGetBuf(short r,long *n){(void)r;*n=input_size;return 0;}
int FSRead(short r,long *n,void *p){(void)r;if(*n>input_size)*n=input_size;memcpy(p,input,*n);memmove(input,input+*n,input_size-*n);input_size-=*n;return 0;}
int PBWriteAsync(ParamBlockRec *p){p->ioParam.ioActCount=p->ioParam.ioReqCount;p->ioParam.ioResult=hang_send?1:0;if(hang_send)pending=p;return 0;}
int KillIO(short r){(void)r;if(delay_abort)return -1;if(pending){pending->ioParam.ioResult=-1;pending=0;}return 0;}
int app_yield(void){return 0;}long cli_write(int fd,const void *p,unsigned long n){(void)fd;(void)p;return n;}
int cli_pause_ticks(unsigned long n){assert(n==1&&driver&&allocations==1);assert(input_open||output_open);if(buf_owned){volatile unsigned char b=driver->ring[4095];(void)b;}cleanup_pauses++;assert(cleanup_pauses<100);tick++;return 1;/* Cancellation cannot abandon cleanup. */}
void app_remote_indicator(int slot,int state){assert(slot==1);(void)state;}
int cli_service_register_slot(int slot,const ClariceService *s){assert(slot==1&&s->poll==poll_service);return register_fail?-1:0;}
int cli_service_register_status_slot(int slot,const ClariceService *s,int (*status)(void)){assert(status==listener_status);return cli_service_register_slot(slot,s);}
int cli_service_stop_slot(int slot){assert(slot==1);return stop();}int cli_service_enabled_slot(int slot){assert(slot==1);return driver!=0;}
int app_remote_settings(ClariceRemoteSettings *s){(void)s;return 0;}int app_serial_remote_settings(ClariceRemoteSettings *s){(void)s;return 0;}
int cli_file_open(const char *p,const char *m){(void)p;(void)m;return 1;}long cli_file_read(int r,void *p,unsigned long n){(void)r;const char *c="CLARICE-SERIAL-1 modem 57600 1000 00000000000000000000000000000000 0000000000000000000000000000000000000000000000000000000000000000\n";assert(n>=strlen(c));memcpy(p,c,strlen(c));return strlen(c);}
long cli_file_write(int r,const void *p,unsigned long n){(void)r;(void)p;return n;}int cli_file_close(int r){(void)r;return 0;}
void *cli_replace_begin(const char *p,char *out,unsigned long n){(void)p;(void)out;(void)n;return 0;}int cli_replace_commit(void *p){(void)p;return -1;}void cli_replace_abort(void *p){(void)p;}
unsigned long cli_mac_time(void){return 1;}
void *cli_session_open(const ClariceSessionIO *io){assert(io->read&&io->write);return (void*)1;}int cli_session_close(void *p){assert(p==(void*)1);return 0;}
int cli_session_set_wire(void *p,ClariceSessionWire w){assert(p==(void*)1&&w==session_wire);return 0;}
void cli_session_prompt(void *p,char *b,unsigned long n){assert(p==(void*)1);snprintf(b,n,"/ > ");}
int cli_session_run(void *p,const char *cmd){assert(p==(void*)1&&!strcmp(cmd,"echo good"));executed++;return 0;}
static void feed(const char *p,unsigned n){assert(n<=sizeof input);memcpy(input,p,n);input_size=n;while(input_size||received_count||used)pump();}
int main(void){
 char *argv[]={"seriald",NULL};
 for(unsigned cycle=0;cycle<20;cycle++)for(unsigned mode=0;mode<3;mode++){
  assert(!allocations&&!input_open&&!output_open&&!buf_owned&&!pending);
  open_fail=mode==0;reset_fail=mode==1;register_fail=mode==2;
  close_in_fail=3;close_out_fail=mode?2:0;detach_fail=mode==2?4:0;cleanup_pauses=0;
  assert(clarice_main(1,argv)==1);
  assert(!driver&&!allocations&&!input_open&&!output_open&&!buf_owned&&!pending);
  assert(!close_in_fail&&!close_out_fail&&!detach_fail&&cleanup_pauses==(mode==0?3:mode==1?5:9));
  assert(!stop());
 }
 open_fail=reset_fail=register_fail=0;
 fail_alloc=1;assert(clarice_main(1,argv)==1&&!driver&&!allocations);fail_alloc=0;
 puts("PASS Seriald startup: partial open/reset/registration failures; repeated detach/close failures; cancellation during retry; 60 eventual teardowns; zero native owners/allocations and idempotent cleanup");
 peer=1;port=57600;protected_login=1;memset(salt,0,sizeof salt);assert(!derive("test",verifier));driver=(DriverStorage*)NewPtrSysClear(sizeof *driver);assert(!listen_again());
 feed("\r",1);assert(connected&&!session);feed("test\r\necho bad\r\n",16);poll_service(1);assert(session&&!ready_used&&!executed);
 feed("echo good\r\n",11);poll_service(1);assert(executed==1);
 running=1;assert(!session_wire(1,0,0));unsigned char data[]={0,1,255,13,10,128},got[6];feed((char*)data,sizeof data);assert(session_wire(2,got,sizeof got)==sizeof got&&!memcmp(data,got,sizeof got));assert(!session_wire(4,0,0));running=0;
 hang_send=1;text("pending");pump();assert(pending);delay_abort=1;assert(stop()&&driver&&buf_owned);delay_abort=0;assert(!stop());assert(!allocations&&!pending);
 puts("PASS native serial login, pre-auth discard, shell input, raw binary, delayed abort ownership and cleanup");
}
