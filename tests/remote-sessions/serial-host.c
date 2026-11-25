#include "../../src/commands/remote/seriald.c"
#include <assert.h>
static unsigned allocations,executed;static int fail_alloc,delay_abort,hang_send,overrun;
static unsigned long tick=1;static ParamBlockRec *pending;static char input[1024];static unsigned input_size;static int input_open,output_open,buf_owned;
Ptr NewPtrSys(unsigned long n){if(fail_alloc)return 0;void *p=malloc(n);if(p)allocations++;return p;}
Ptr NewPtrSysClear(unsigned long n){Ptr p=NewPtrSys(n);if(p)memset(p,0,n);return p;}
void DisposePtr(Ptr p){if(p==(Ptr)driver)assert(!input_open&&!output_open&&!buf_owned&&!pending);allocations--;free(p);}
void *cli_alloc(unsigned long n){return NewPtrSys(n);}void cli_free(void *p){DisposePtr(p);}
unsigned long TickCount(void){return tick++;}short MemError(void){return -108;}
short OpenDriver(ConstStringPtr name,short *ref){if(name[0]==4){input_open=1;*ref=1;}else{output_open=1;*ref=2;}return 0;}
int CloseDriver(short ref){if(ref==1)input_open=0;else output_open=0;return 0;}
int SerSetBuf(short ref,Ptr p,long size){(void)ref;(void)size;buf_owned=p!=0;return 0;}
int SerReset(short r,int s){(void)r;(void)s;return 0;}int SerHShake(short r,SerShk *s){(void)r;assert(!s->fXOn&&!s->fCTS);return 0;}
int SerStatus(short r,SerStaRec *s){(void)r;s->cumErrs=overrun;return 0;}
int SerGetBuf(short r,long *n){(void)r;*n=input_size;return 0;}
int FSRead(short r,long *n,void *p){(void)r;if(*n>input_size)*n=input_size;memcpy(p,input,*n);memmove(input,input+*n,input_size-*n);input_size-=*n;return 0;}
int PBWriteAsync(ParamBlockRec *p){p->ioParam.ioActCount=p->ioParam.ioReqCount;p->ioParam.ioResult=hang_send?1:0;if(hang_send)pending=p;return 0;}
int KillIO(short r){(void)r;if(delay_abort)return -1;if(pending){pending->ioParam.ioResult=-1;pending=0;}return 0;}
int app_yield(void){return 0;}long cli_write(int fd,const void *p,unsigned long n){(void)fd;(void)p;return n;}
void app_remote_indicator(int slot,int state){assert(slot==1);(void)state;}
int cli_service_register_slot(int slot,const ClariceService *s){assert(slot==1&&s->poll==poll_service);return 0;}
int cli_service_register_status_slot(int slot,const ClariceService *s,int (*status)(void)){assert(status==listener_status);return cli_service_register_slot(slot,s);}
int cli_service_stop_slot(int slot){assert(slot==1);return stop();}int cli_service_enabled_slot(int slot){assert(slot==1);return driver!=0;}
int app_remote_settings(ClariceRemoteSettings *s){(void)s;return 0;}int app_serial_remote_settings(ClariceRemoteSettings *s){(void)s;return 0;}
int cli_file_open(const char *p,const char *m){(void)p;(void)m;return -1;}long cli_file_read(int r,void *p,unsigned long n){(void)r;(void)p;(void)n;return -1;}
long cli_file_write(int r,const void *p,unsigned long n){(void)r;(void)p;return n;}int cli_file_close(int r){(void)r;return 0;}
void *cli_replace_begin(const char *p,char *out,unsigned long n){(void)p;(void)out;(void)n;return 0;}int cli_replace_commit(void *p){(void)p;return -1;}void cli_replace_abort(void *p){(void)p;}
unsigned long cli_mac_time(void){return 1;}
void *cli_session_open(const ClariceSessionIO *io){assert(io->read&&io->write);return (void*)1;}int cli_session_close(void *p){assert(p==(void*)1);return 0;}
int cli_session_set_wire(void *p,ClariceSessionWire w){assert(p==(void*)1&&w==session_wire);return 0;}
void cli_session_prompt(void *p,char *b,unsigned long n){assert(p==(void*)1);snprintf(b,n,"/ > ");}
int cli_session_run(void *p,const char *cmd){assert(p==(void*)1&&!strcmp(cmd,"echo good"));executed++;return 0;}
static void feed(const char *p,unsigned n){assert(n<=sizeof input);memcpy(input,p,n);input_size=n;while(input_size||received_count||used)pump();}
int main(void){
 peer=1;port=57600;protected_login=1;memset(salt,0,sizeof salt);assert(!derive("test",verifier));driver=(DriverStorage*)NewPtrSysClear(sizeof *driver);assert(!listen_again());
 feed("\r",1);assert(connected&&!session);feed("test\r\necho bad\r\n",16);poll_service(1);assert(session&&!ready_used&&!executed);
 feed("echo good\r\n",11);poll_service(1);assert(executed==1);
 running=1;assert(!session_wire(1,0,0));unsigned char data[]={0,1,255,13,10,128},got[6];feed((char*)data,sizeof data);assert(session_wire(2,got,sizeof got)==sizeof got&&!memcmp(data,got,sizeof got));assert(!session_wire(4,0,0));running=0;
 hang_send=1;text("pending");pump();assert(pending);delay_abort=1;assert(stop()&&driver&&buf_owned);delay_abort=0;assert(!stop());assert(!allocations&&!pending);
 puts("PASS native serial login, pre-auth discard, shell input, raw binary, delayed abort ownership and cleanup");
}
