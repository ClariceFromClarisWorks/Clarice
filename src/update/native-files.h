/* Small native backend for the independent updater; no Unix namespace.
 * Uses the same native catalog/fork/rename operations as Clarice's HFS runtime.
 * Every path must name exactly one mounted volume. No default-volume fallback.
 */
#ifndef CLARICE_UPDATE_NATIVE_FILES_H
#define CLARICE_UPDATE_NATIVE_FILES_H
#include <Files.h>
#include <Errors.h>
#include <string.h>
static inline int update_volume(const char *path,short *volume){
 const char *end=strchr(path,':');if(!end||end==path||end-path>31||strlen(path)>255)return bdNamErr;
 unsigned char requested[32];requested[0]=end-path;memcpy(requested+1,path,requested[0]);
 int hits=0,e;short found=0;
 for(int i=1;;i++){HParamBlockRec p;unsigned char name[256];memset(&p,0,sizeof p);p.volumeParam.ioNamePtr=name;p.volumeParam.ioVolIndex=i;e=PBHGetVInfoSync(&p);if(e)break;
  if(EqualString(name,requested,0,1)){hits++;found=p.volumeParam.ioVRefNum;}
 }
 if(e!=nsvErr)return e;if(hits!=1||!found)return hits?dupFNErr:nsvErr;*volume=found;return 0;
}
static inline void update_pstr(unsigned char *p,const char *s){p[0]=strlen(s);memcpy(p+1,s,p[0]);}
static inline int update_info(const char *path,CInfoPBRec *p){short v;int e=update_volume(path,&v);if(e)return e;unsigned char n[256];update_pstr(n,path);memset(p,0,sizeof *p);p->hFileInfo.ioNamePtr=n;p->hFileInfo.ioVRefNum=v;return PBGetCatInfoSync(p);}
static inline int update_open(const char *path,int resource,int write,short *ref){short v;int e=update_volume(path,&v);if(e)return e;unsigned char n[256];update_pstr(n,path);return resource?HOpenRF(v,0,n,write?fsWrPerm:fsRdPerm,ref):HOpen(v,0,n,write?fsWrPerm:fsRdPerm,ref);}
static inline int update_flush(const char *path){short v;int e=update_volume(path,&v);return e?e:FlushVol(0,v);}
static inline int update_rename(const char *a,const char *b){
 const char *x=strrchr(a,':'),*y=strrchr(b,':');if(!x||!y||x-a!=y-b||memcmp(a,b,x-a)||!x[1]||!y[1]||strlen(y+1)>31)return bdNamErr;
 CInfoPBRec p;int e=update_info(b,&p);if(e!=fnfErr)return e?e:dupFNErr;
 e=update_info(a,&p);if(e)return e;short v;e=update_volume(a,&v);if(e)return e;
 unsigned char old[256],next[256];update_pstr(old,x+1);update_pstr(next,y+1);return HRename(v,p.hFileInfo.ioFlParID,old,next);
}
#endif
