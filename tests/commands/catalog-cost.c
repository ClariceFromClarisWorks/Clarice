/* Read-only Snow/native HFS diagnostic, not a bundled command.
 * Compare documented synchronous/asynchronous catalog calls with a hot-node
 * control. Explicit volume and parent are restored on every invocation.
 * Async storage stays live until completion, including after cancellation. */
#include "../../src/sys6.h"
#include <Files.h>
#include <Errors.h>
#include <OSUtils.h>
#include <string.h>
#include <stdio.h>
extern int resolve(const char *,char *),info(const char *,CInfoPBRec *);
extern void say(const char *),error(const char *,int);
typedef struct {unsigned long ticks,hash,waits,begin,end;int count,error;} Sample;
static unsigned long mix(unsigned long h,unsigned long v){return (h<<5)+(h>>27)+v;}
int clarice_main(int argc,char **argv){
 char native[PATH_CAP],report[220];CInfoPBRec dir,pb;unsigned char name[256];
 Sample samples[9];int e,mode,run,j,cancel=0;short volume;long parent;
 if(argc!=2){say("catalog-cost directory: three sync, async and hot-index scans\n");return 2;}
 e=resolve(argv[1],native);if(!e)e=info(native,&dir);
 if(e){error("catalog-cost",e);return 1;}
 if(!(dir.hFileInfo.ioFlAttrib&16)){say("catalog-cost: directory required\n");return 1;}
 volume=dir.dirInfo.ioVRefNum;parent=dir.dirInfo.ioDrDirID;
 memset(samples,0,sizeof samples);
 for(mode=0;mode<3;mode++)for(run=0;run<3;run++){
  Sample *s=&samples[mode*3+run];memset(&pb,0,sizeof pb);s->begin=TickCount();
  for(j=1;j<=1001;j++){
   if(mode==2&&j>1000)break;
   pb.hFileInfo.ioNamePtr=name;pb.hFileInfo.ioVRefNum=volume;
   pb.hFileInfo.ioDirID=parent;pb.hFileInfo.ioFDirIndex=mode==2?1:j;
   if(mode==1){
    e=PBGetCatInfoAsync(&pb);
    while(pb.hFileInfo.ioResult>0){s->waits++;if(app_yield())cancel=1;}
    if(!e)e=pb.hFileInfo.ioResult;
   }else e=PBGetCatInfoSync(&pb);
   if(e==fnfErr)break;
   if(e){s->error=e;break;}
   if(name[0]>31){s->error=bdNamErr;break;}
   s->count++;s->hash=mix(s->hash,pb.hFileInfo.ioFlAttrib);
   s->hash=mix(s->hash,pb.hFileInfo.ioDirID);
   s->hash=mix(s->hash,pb.hFileInfo.ioFlFndrInfo.fdFlags);
   s->hash=mix(s->hash,pb.hFileInfo.ioFlCrDat);
   if(!(pb.hFileInfo.ioFlAttrib&16)){
    s->hash=mix(s->hash,pb.hFileInfo.ioFlLgLen);
    s->hash=mix(s->hash,pb.hFileInfo.ioFlRLgLen);
   }
   int k;for(k=0;k<=name[0];k++)s->hash=mix(s->hash,name[k]);
   if(app_yield())cancel=1;if(cancel){s->error=userCanceledErr;break;}
  }
  s->end=TickCount();s->ticks=s->end-s->begin;
  if(s->error)goto finished;
 }
finished:
 for(j=0;j<9;j++){
  Sample *s=&samples[j];if(!s->begin)break;
  run=j;while(run>=3)run-=3;
  snprintf(report,sizeof report,"mode=%s run=%d count=%d ticks=%lu hash=%08lx waits=%lu error=%d begin=%lu end=%lu\n",
   j<3?"sync":j<6?"async":"hot",run,s->count,s->ticks,s->hash,s->waits,s->error,s->begin,s->end);say(report);
  if(s->error)return cancel?130:1;
 }
 return 0;
}
