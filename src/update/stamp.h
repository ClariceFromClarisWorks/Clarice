#ifndef CLARICE_UPDATE_STAMP_H
#define CLARICE_UPDATE_STAMP_H
#include <Resources.h>
#include <Memory.h>
#include "package.h"
#include "identity.h"
#include "native-files.h"
static int update_stamp(const char *path,uint32_t *build,unsigned *system){
 unsigned char p[256];size_t n=strlen(path);if(!n||n>255)return -37;p[0]=n;memcpy(p+1,path,n);
 short old=CurResFile(),ref=-1;int owned=1;char own[256];
 /* Opening an already-open application resource file may return its existing
    reference. Never close the running application's resource file. */
 if(!update_application_path(own)&&!strcmp(own,path)){unsigned char name[256];Handle parms;GetAppParms(name,&ref,&parms);owned=0;}
 else {short volume;int e=update_volume(path,&volume);if(e)return e;
  /* System 6 fallback. Disable CODE preloading while opening another app. */
  SetResLoad(0);ref=OpenRFPerm(p,volume,fsRdPerm);int opened=ResError();SetResLoad(1);
  if(ref<0)return opened?opened:resNotFound;
 }
 if(ref<0){int e=ResError();return e?e:resNotFound;}
 UseResFile(ref);Handle h=Get1Resource('CLUP',128);int e=0;
 if(!h)e=resNotFound;
 else {if(GetHandleSize(h)!=16)e=paramErr;else {HLock(h);const unsigned char *b=(const unsigned char*)*h;
   if(memcmp(b,"CLU1",4)||b[10]||b[11]!=1||b[12]||b[13]!=1||b[14]||b[15]>2)e=paramErr;
   else {*build=update_be32(b+4);*system=((unsigned)b[8]<<8)|b[9];if(!*build||*system<0x600)e=paramErr;}
   HUnlock(h);}
  ReleaseResource(h);}
 int close_error=0;if(owned){CloseResFile(ref);close_error=ResError();}UseResFile(old);return e?e:close_error;
}
#endif
