/* Disposable Snow probe: real Serial Driver ownership, injected failures before
 * selected native calls. Config/registration fixtures never enable a service. */
#include <Multiverse.h>
#include <Serial.h>
#include <Devices.h>
#include <Memory.h>
#include <string.h>
#include <stdio.h>
#include "../../src/service.h"
extern int cli_pause_ticks(unsigned long);
extern void say(const char *);
static int mode,close_in,close_out,detach,pauses,opened_in,opened_out,attached;
static short in_ref;
int test_register(int,const ClariceService *,int (*)(void));
static OSErr test_open(ConstStringPtr n,short *r){
 if(n[0]==5&&mode==0)return -36;
 OSErr e=OpenDriver(n,r);if(!e){if(n[0]==4){opened_in=1;in_ref=*r;}else opened_out=1;}return e;
}
static OSErr test_reset(short r,short s){return mode==1?-36:SerReset(r,s);}
static OSErr test_buf(short r,Ptr p,long n){if(!p&&detach){detach--;return -36;}OSErr e=SerSetBuf(r,p,n);if(!e)attached=p!=0;return e;}
static OSErr test_close(short r){int *f=r==in_ref?&close_in:&close_out;if(*f){--*f;return -36;}OSErr e=CloseDriver(r);if(!e){if(r==in_ref)opened_in=0;else opened_out=0;}return e;}
static int test_pause(unsigned long n){pauses++;(void)cli_pause_ticks(n);return 1;}
#define OpenDriver test_open
#define SerReset test_reset
#define SerSetBuf test_buf
#define CloseDriver test_close
#define cli_pause_ticks test_pause
#define cli_service_register_status_slot test_register
#define cli_file_open test_file_open
#define cli_file_read test_file_read
#define cli_file_close test_file_close
#define clarice_main service_main
#include "../../src/commands/remote/seriald.c"
#undef clarice_main
int test_register(int slot,const ClariceService *s,int (*status)(void)){(void)slot;(void)s;(void)status;return -1;}
int test_file_open(const char *p,const char *m){(void)p;(void)m;return 1;}
long test_file_read(int f,void *p,unsigned long n){(void)f;const char *s="CLARICE-SERIAL-1 modem 57600 1000 00000000000000000000000000000000 0000000000000000000000000000000000000000000000000000000000000000\n";if(n<strlen(s))return -1;memcpy(p,s,strlen(s));return strlen(s);}
int test_file_close(int f){(void)f;return 0;}
static long sys_free(void){THz old=GetZone();SetZone(LMGetSysZone());long n=FreeMem();SetZone(old);return n;}
int clarice_main(int argc,char **argv){(void)argc;(void)argv;char *av[]={"seriald",0};char out[192];
 /* Opening native drivers may cause one-time system resource allocations. */
 for(int cycle=0;cycle<5;cycle++)for(mode=0;mode<3;mode++){
  long before=sys_free(),app=FreeMem();close_in=3;close_out=mode?2:0;detach=mode==2?4:0;pauses=0;
  int rc=service_main(1,av);long delta=sys_free()-before,adelta=FreeMem()-app;
  int good=rc==1&&!driver&&!opened_in&&!opened_out&&!attached&&!close_in&&!close_out&&!detach&&pauses==(mode==0?3:mode==1?5:9);
  if(good)good=!stop();
  snprintf(out,sizeof out,"SERIAL_START cycle=%d mode=%d rc=%d pauses=%d system_delta=%ld app_delta=%ld safe=%d\n",cycle,mode,rc,pauses,delta,adelta,good);say(out);
  if(!good||adelta||(cycle&&delta))return 1;
 }
 say("PASS native startup/registration retry, repeated teardown and stable heaps\n");return 0;
}
