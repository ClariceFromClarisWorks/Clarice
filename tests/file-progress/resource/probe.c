/* Native System 6 Resource Manager acceptance probe; not a shipped command. */
#include "../../../src/sys6.h"
#include <Files.h>
#include <Resources.h>
#include <Memory.h>
#include <string.h>
#include <stdio.h>
extern int cli_resolve_info(const char *,char *,char *,CInfoPBRec *);
extern void say(const char *);
struct expected {unsigned long type;short id,attrs;long size;unsigned long hash;const char *name;};
static const struct expected values[]={
#include "../../../work/file-progress/resource/expected.h"
};
int clarice_main(int argc,char **argv){
 if(argc!=2)return 2;char path[PATH_CAP];CInfoPBRec pb;unsigned char p[256];int e=cli_resolve_info(argv[1],path,0,&pb);if(e)return 1;
 p[0]=strlen(path);memcpy(p+1,path,p[0]);short old=CurResFile();short ref=OpenRFPerm(p,0,fsRdPerm);if(ref==-1||ResError())return 1;
 UseResFile(ref);int ok=1;
 for(unsigned i=0;i<3&&ok;i++){
  const struct expected *x=&values[i];Handle h=GetResource(x->type,x->id);if(!h||ResError()||HomeResFile(h)!=ref){char msg[100];snprintf(msg,sizeof msg,"resource %u missing/error %d/home %d expected %d\n",i,ResError(),h?HomeResFile(h):-1,ref);say(msg);ok=0;break;}
  /* Resource attributes are an 8-bit field; this trap/toolchain can leave
   * the high byte of its short result unspecified on System 6. */
  short id;ResType type;Str255 name;GetResInfo(h,&id,&type,name);
  ok=!ResError()&&id==x->id&&(unsigned long)type==x->type&&name[0]==strlen(x->name)&&!memcmp(name+1,x->name,name[0])&&(unsigned char)GetResAttrs(h)==x->attrs&&GetHandleSize(h)==x->size;
  unsigned long hash=2166136261UL;HLock(h);for(long j=0;j<x->size&&ok;j++)hash=(hash^(unsigned char)(*h)[j])*16777619UL;HUnlock(h);if(hash!=x->hash)ok=0;if(!ok){char msg[200];snprintf(msg,sizeof msg,"resource %u: id=%d type=%08lx name=%.*s attrs=%d size=%ld hash=%08lx expected=%08lx\n",i,id,(unsigned long)type,(int)name[0],name+1,(unsigned char)GetResAttrs(h),GetHandleSize(h),hash,x->hash);say(msg);}ReleaseResource(h);
 }
 CloseResFile(ref);if(ResError())ok=0;UseResFile(old);say(ok?"PASS native resource types/IDs/names/attributes/sizes/payloads\n":"FAIL native resources\n");return ok?0:1;
}
