/* Native System 6 transfer launch, following Apple TN126's LC parameter block.
 * Called only after caller-owned I/O and application cleanup. Successful launch
 * does not return. No trap patching, event injection or background worker. */
#ifndef CLARICE_UPDATE_NATIVE_LAUNCH_H
#define CLARICE_UPDATE_NATIVE_LAUNCH_H
#include "native-files.h"
#include <Errors.h>
#include <string.h>
#ifndef UPDATE_LAUNCH_TRACE
#define UPDATE_LAUNCH_TRACE(s) ((void)0)
#endif
static int update_native_launch(const char *path){
 UPDATE_LAUNCH_TRACE("native launch entry");UPDATE_LAUNCH_TRACE(path);unsigned char name[256];size_t n=strlen(path);CInfoPBRec pb;
 if(!n||n>255||!strchr(path,':')||path[0]==':')return bdNamErr;
 name[0]=n;memcpy(name+1,path,n);
 UPDATE_LAUNCH_TRACE("catalog");int e=update_info(path,&pb);if(e)return e;
 if(pb.hFileInfo.ioFlAttrib&16||pb.hFileInfo.ioFlFndrInfo.fdType!='APPL')return paramErr;
 /* TN126 uses SetVol with a working-directory reference, not merely a
    volume/directory pair. Keep that WD open across transfer launch. */
 short ref=-1,oldvol=0;FCBPBRec identity;WDPBRec wd;
 UPDATE_LAUNCH_TRACE("resource FCB");e=HOpenRF(0,0,name,fsRdPerm,&ref);if(e)return e;
 memset(&identity,0,sizeof identity);identity.ioRefNum=ref;e=PBGetFCBInfoSync(&identity);
 int closed=FSClose(ref);if(e||closed)return e?e:closed;
 UPDATE_LAUNCH_TRACE("working directory");e=GetVol(0,&oldvol);if(e)return e;
 memset(&wd,0,sizeof wd);wd.ioVRefNum=identity.ioFCBVRefNum;
 wd.ioWDDirID=identity.ioFCBParID;wd.ioWDProcID='CLUP';
 e=PBOpenWDSync(&wd);if(e)return e;
 short targetwd=wd.ioVRefNum;e=SetVol(0,targetwd);
 if(e){PBCloseWDSync(&wd);return e;}
 const char *leaf=strrchr(path,':')+1;name[0]=strlen(leaf);memcpy(name+1,leaf,name[0]);
 struct __attribute__((packed,aligned(2))) {unsigned char *name;short param;char lc[2];long length;short flags;unsigned long options;} block={name,0,{'L','C'},6,pb.hFileInfo.ioFlFndrInfo.fdFlags,0};
 UPDATE_LAUNCH_TRACE("Launch trap");register void *a0 __asm__("a0")=&block;register long d0 __asm__("d0");
 __asm__ volatile(".word 0xa9f2":"=d"(d0),"+a"(a0)::"d1","a1","cc","memory");
 int result=(short)d0;UPDATE_LAUNCH_TRACE("Launch returned");SetVol(0,oldvol);wd.ioVRefNum=targetwd;PBCloseWDSync(&wd);return result;
}
#endif
