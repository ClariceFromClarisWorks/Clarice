/* Disposable guest diagnostic. Native allocations deliberately bypass cli_alloc
   to exercise the loader's existing reserve under cumulative/fragmented pressure. */
#include <Memory.h>
#include <stdio.h>
#include "record.h"
#include "../../src/session.h"
extern void say(const char *);
static long read_io(void*c,void*b,unsigned long n){(void)c;(void)b;(void)n;return 0;}
/* Do not call say here: it routes back through this active session callback.
   The child records its entry measurements using native file I/O. */
static long write_io(void*c,int e,const void*b,unsigned long n){(void)c;(void)e;(void)b;return n;}
static int cancel_io(void*c){(void)c;return 0;}
int clarice_main(int argc,char**argv){
 if(argc<2)return 2;
 ClariceSessionIO io={0,read_io,write_io,cancel_io};void*session=cli_session_open(&io);if(!session)return 3;
 Ptr held[128]={0};int count=0;char line[180];long initial=FreeMem(),initial_largest=MaxBlock();
 while(count<128&&FreeMem()>85000){held[count]=NewPtr(8192);if(!held[count])break;count++;}
 if(argc>2)for(int i=0;i<count;i+=2){DisposePtr(held[i]);held[i]=0;}
 snprintf(line,sizeof line,"pressure before free=%ld largest=%ld blocks=%d\n",FreeMem(),MaxBlock(),count);record("/mnt/Sys6Unix/Perf/parent.txt",line);say(line);
 int rc=cli_session_run(session,argv[1]);
 for(int i=0;i<count;i++)if(held[i])DisposePtr(held[i]);
 snprintf(line,sizeof line,"pressure result=%d freed=%ld/%ld largest=%ld/%ld\n",rc,FreeMem(),initial,MaxBlock(),initial_largest);say(line);
 cli_session_close(session);return rc&255;
}
