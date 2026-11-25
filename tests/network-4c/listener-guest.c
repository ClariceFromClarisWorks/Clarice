/* Disposable Snow-only probe. Real .IPP ownership/IO, except TCPCreate returns
 * -36 for ten seconds after the first connection. Not shipped or installed on
 * hardware. The normal service is tested separately. */
#include <Multiverse.h>
static unsigned creates;
static unsigned long failing_since;
static OSErr fault_control(ParmBlkPtr b){TCPiopb *p=(TCPiopb*)b;
 if(p->csCode==TCPCreate&&creates++){
  if(!failing_since)failing_since=TickCount();
  if(TickCount()-failing_since<600)return -36;
 }
 return PBControlSync(b);
}
#define PBControlSync fault_control
#include "../../src/commands/remote/telnetd.c"
