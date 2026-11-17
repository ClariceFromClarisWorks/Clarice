/* Test-only CLX: real asynchronous Serial Driver request, deterministic cancel
   before completion. Production serial.c is included unchanged. */
#include "../../src/sys6.h"
#include <Serial.h>
#include <Devices.h>
#include <Files.h>
#include <Errors.h>
#include <Memory.h>
#include <stdio.h>
#include <string.h>
extern long cli_write(int,const void *,unsigned long);
static int armed,keys,pending_seen,kill_result,after_kill,hold_result;
static ParamBlockRec *request;
static int test_yield(void){if(armed){pending_seen=((volatile ParamBlockRec*)request)->ioParam.ioResult>0;return 1;}return app_yield();}
static int test_key(void){return keys++<63?'x':-1;}
static int test_tty(int fd){(void)fd;return 1;}
static int test_write(ParamBlockRec *pb){SerShk handshake;memset(&handshake,0,sizeof handshake);handshake.fXOn=1;handshake.xOn=17;handshake.xOff=19;SerHShake(pb->ioParam.ioRefNum,&handshake);ParamBlockRec hold;memset(&hold,0,sizeof hold);hold.cntrlParam.ioCRefNum=pb->ioParam.ioRefNum;hold.cntrlParam.csCode=21;hold_result=PBControlSync(&hold);int e=PBWriteAsync(pb);request=pb;armed=1;return e;}
static int test_kill(short ref){int e=KillIO(ref);kill_result=e;after_kill=((volatile ParamBlockRec*)request)->ioParam.ioResult;return e;}
#define SERIAL_HOST_TEST
#define clarice_main serial_main
#define app_yield test_yield
#define app_serial_key test_key
#define cli_isatty test_tty
#define PBWriteAsync test_write
#define KillIO test_kill
#include "../../src/commands/serial.c"
#undef clarice_main
int clarice_main(int argc,char **argv){char b[180];int failed=0;(void)argc;(void)argv;
 for(int i=0;i<3;i++){
  char *args[]={"serial","-b","57600","/dev/tty.modem"};
  armed=keys=pending_seen=0;kill_result=after_kill=999;
  long before=FreeMem();int rc=serial_main(4,args);long delta=FreeMem()-before;
  snprintf(b,sizeof b,"cancel %d: rc=%d pending=%d hold=%d KillIO=%d ioResult=%d heap_delta=%ld\n",i,rc,pending_seen,hold_result,kill_result,after_kill,delta);cli_write(1,b,strlen(b));
  if(rc!=130||!pending_seen||kill_result||after_kill>0||delta)failed++;
 }
 return failed?1:0;
}
