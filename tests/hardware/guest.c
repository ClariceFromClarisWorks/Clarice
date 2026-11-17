/* Test-only consumer of the independent runtime 1.3 capability snapshot. */
#include <Multiverse.h>
#include <stdio.h>
#include <string.h>
#include "../../src/hardware.h"
extern unsigned long cli_ticks(void);
extern void say(const char *);
int clarice_main(int argc,char **argv){(void)argc;(void)argv;ClariceHardware h;char b[256];
 int e=cli_hardware(&h,sizeof h);if(e)return 1;
 snprintf(b,sizeof b,"Hardware v%u: %s; CPU=%u FPU=%d RAM=%lu\nDisplay=%ux%ux%u; System API=%x; model=%ld namespace=%d\n",h.version,h.model_name,h.cpu,h.fpu,(unsigned long)h.ram,h.width,h.height,h.depth,h.system,(long)h.model,h.model_namespace);say(b);
 long before=FreeMem();unsigned long t=cli_ticks();for(int i=0;i<20;i++)if(cli_hardware(&h,sizeof h))return 1;
 snprintf(b,sizeof b,"20 capability queries: %lu ticks; retained heap=%ld\n",cli_ticks()-t,before-FreeMem());say(b);
 memset(&h,0xa5,sizeof h);if(cli_hardware(&h,sizeof h-1)!=-50||h.version!=0xa5a5||cli_hardware(0,sizeof h)!=-50)return 1;
 say("CAPABILITIES PASS\n");return 0;
}
