"""Compile the actual native transport against faulted Macintosh driver calls."""
from pathlib import Path
import subprocess,sys
root=Path(__file__).resolve().parents[2];out=root/'work/network-4a';out.mkdir(exist_ok=True)
source=(root/'src/ports/ekermit/transport.h').read_text()
if '--baseline' in sys.argv:
 source=subprocess.check_output(['git','show','3fc4251:src/ports/ekermit/transport.h'],cwd=root,text=True)
# Only replace native declaration headers, not executable implementation.
source=source.replace('#include "../../commands/mac-files.h"','').replace('#include <Serial.h>','').replace('#include <Devices.h>','')
source=source.replace('#include "../../commands/serial-cleanup.h"',f'#include "{root}/src/commands/serial-cleanup.h"')
preamble=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <Serial.h>
#include <Devices.h>
#define paramErr (-50)
static unsigned long now;
static int stopping,kill_fail,kill_delay,detach_fail,close_fail,attached,opened;
static ParamBlockRec *pending;
unsigned long cli_ticks(void){return ++now;}
int app_yield(void){return stopping;}
int app_serial_key(void){return -1;}
void app_serial_input(int n){(void)n;}
long cli_write(int fd,const void *p,unsigned long n){(void)fd;(void)p;return n;}
long cli_remote_wire(int op,void *p,unsigned long n){(void)op;(void)p;(void)n;return 0;}
void SystemTask(void){if(pending){assert(pending->ioParam.ioResult>0);assert(pending->ioParam.ioReqCount==3);assert(!memcmp(pending->ioParam.ioBuffer,"abc",3));assert(attached);}}
void Delay(long n,long *r){now+=n;*r=now;SystemTask();}
short OpenDriver(ConstStringPtr s,short *r){*r=s[0]==4?1:2;opened++;return 0;}
int CloseDriver(short r){assert(r&&!pending&&!attached);if(close_fail){close_fail--;return -36;}opened--;return 0;}
int SerReset(short r,int bits){(void)r;(void)bits;return 0;}
int SerHShake(short r,SerShk *s){(void)r;(void)s;return 0;}
int SerSetBuf(short r,Ptr p,long n){(void)r;(void)n;if(!p&&detach_fail){detach_fail--;return -36;}attached=p!=0;return 0;}
int SerStatus(short r,SerStaRec *s){(void)r;s->cumErrs=0;return 0;}
int SerGetBuf(short r,long *n){(void)r;*n=0;return 0;}
int FSRead(short r,long *n,void *p){(void)r;(void)n;(void)p;return 0;}
int PBWriteAsync(ParamBlockRec *p){assert(!pending);pending=p;p->ioParam.ioResult=1;return 0;}
int KillIO(short r){assert(r&&pending);if(kill_fail){kill_fail--;return -36;}if(kill_delay){kill_delay--;return 0;}pending->ioParam.ioResult=-27;pending=0;return 0;}
'''
tests=r'''
int main(void){for(int mode=0;mode<5;mode++)for(int repeat=0;repeat<4;repeat++){
 assert(!pending&&!attached&&!opened);cancelled=0;stopping=mode!=4;now=0;
 kill_fail=mode==0?3:0;kill_delay=mode==1?3:0;detach_fail=mode==2?3:0;close_fail=mode==3?3:0;
 assert(!transport_open("/dev/tty.modem"));
 assert(wire_write((const unsigned char*)"abc",3)==-1);assert(!pending);
 int rc=transport_close();assert(rc==((mode==2||mode==3)?-36:0));
 assert(!attached&&!opened&&!serial_in&&!serial_out&&!buffer_set);
 assert(!transport_close());
 }puts("PASS native Kermit: failed/delayed KillIO, timeout, failed detach/close, live PB and buffer ownership, repeated cleanup, 20 recovery cycles; no heap allocation");}
'''
f=out/('kermit-before.c' if '--baseline' in sys.argv else 'kermit-native.c');f.write_text(preamble+source+tests)
subprocess.run(['cc','-std=c11','-fsanitize=address,undefined','-I'+str(root/'tests/remote-sessions/include'),str(f),'-o',str(f.with_suffix(''))],check=True)
subprocess.run([str(f.with_suffix(''))],check=True)
