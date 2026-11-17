/* Diagnostic only: inspect shared newlib FILE reuse without altering its internals.
   Cookie streams avoid the native FCB limit. No protocol/production changes. */
#include <Memory.h>
#include <stdio.h>
#include <stdint.h>
extern void say(const char *);
static int empty(void *cookie,char *out,int n){(void)cookie;(void)out;(void)n;return 0;}
int clarice_main(int argc,char **argv){
 (void)argc;(void)argv;FILE *streams[32];uintptr_t previous[32];char line[160];
 long baseline=FreeMem();int failed=0;
 for(int run=0;run<3;run++){
  int count=0,reused=0;long before=FreeMem();
  for(;count<32;count++){streams[count]=funopen(0,empty,0,0,0);if(!streams[count]){failed=1;break;}
   if(run)for(int j=0;j<32;j++)if(previous[j]==(uintptr_t)streams[count]){reused++;break;}}
  long opened=FreeMem();
  for(int i=0;i<count;i++){previous[i]=(uintptr_t)streams[i];if(fclose(streams[i]))failed=1;}
  long after=FreeMem();
  snprintf(line,sizeof line,"pool run=%d count=%d sizeofFILE=%lu before=%ld open=%ld after=%ld reused=%d retained=%ld\n",run,count,(unsigned long)sizeof(FILE),before,opened,after,reused,baseline-after);say(line);
  if(count!=32||(run&&reused!=32))failed=1;
 }
 return failed;
}
