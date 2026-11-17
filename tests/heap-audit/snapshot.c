/* Disposable native diagnostic; no production allocator changes.
   24-bit block layout cross-checked against historical Memory Manager layout
   and metamage_1/68k/modules/ams-core/Zones.cc. Independent read-only walker;
   no code copied. Zone counters are recorded but are zero on this fixture. */
#include <Memory.h>
#include <stdio.h>
extern void say(const char *);
int clarice_main(int argc,char **argv){
 (void)argc;(void)argv;
 THz z=GetZone();long before=FreeMem(),largest=MaxBlock();
 int rel=z->cntRel,nrel=z->cntNRel,empty=z->cntEmpty,handles=z->cntHandles;
 long compact=CompactMem(0x7fffffffL),after=FreeMem();
 char line[240];snprintf(line,sizeof line,"heap=%ld largest=%ld compact=%ld after=%ld rel=%d ptr=%d empty=%d handles=%d\n",before,largest,compact,after,rel,nrel,empty,handles);say(line);
 /* Read-only diagnostic for the known 24-bit Plus fixture. Reject unknown
    layout rather than follow a damaged block chain. Never used by runtime. */
 unsigned char *p=(unsigned char*)&z->heapData,*end=(unsigned char*)z->bkLim;
 unsigned counts[4]={0};unsigned long sizes[4]={0};int valid=1;
 while(p<end){unsigned long header=*(unsigned long*)p,n=header&0xffffffUL;unsigned kind=header>>30;
  if(n<8||(n&1)||n>(unsigned long)(end-p)){valid=0;break;}
  counts[kind]++;sizes[kind]+=n;
  if(kind){snprintf(line,sizeof line,"block %lx %lu %u %lx\n",(unsigned long)p,n,kind,*(unsigned long*)(p+4));say(line);}
  p+=n;
 }
 snprintf(line,sizeof line,"walk valid=%d free=%u/%lu ptr=%u/%lu handle=%u/%lu\n",valid,counts[0],sizes[0],counts[1],sizes[1],counts[2],sizes[2]);say(line);return 0;
}
