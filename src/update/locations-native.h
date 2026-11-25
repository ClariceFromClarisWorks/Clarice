#ifndef CLARICE_LOCATIONS_NATIVE_H
#define CLARICE_LOCATIONS_NATIVE_H
#ifndef LOCATIONS_SYSTEM_PATH
#include <OSUtils.h>
#endif
#include "locations.h"
#include "native-files.h"
#ifndef LOCATIONS_SYSTEM_PATH
/* File Manager catalog identities reconstruct a native directory, including
 * names such as '.' or '/' which must never pass through the Unix resolver. */
static int locations_directory(short volume,long id,char out[256]){
 HParamBlockRec vp;CInfoPBRec c;unsigned char name[256];char tail[256]="",part[256];
 memset(&vp,0,sizeof vp);vp.volumeParam.ioVRefNum=volume;vp.volumeParam.ioNamePtr=name;
 int e=PBHGetVInfoSync(&vp);if(e)return e;
 memcpy(out,name+1,name[0]);out[name[0]]=':';out[name[0]+1]=0;
 for(int depth=0;id>2&&depth<32;depth++){
  memset(&c,0,sizeof c);c.dirInfo.ioNamePtr=name;c.dirInfo.ioVRefNum=volume;c.dirInfo.ioDrDirID=id;c.dirInfo.ioFDirIndex=-1;
  e=PBGetCatInfoSync(&c);if(e)return e;
  if(name[0]+strlen(tail)+2>sizeof part)return bdNamErr;
  memcpy(part,name+1,name[0]);part[name[0]]=':';strcpy(part+name[0]+1,tail);strcpy(tail,part);id=c.dirInfo.ioDrParID;
 }
 if(id!=2||strlen(out)+strlen(tail)>255)return bdNamErr;strcat(out,tail);return 0;
}
static int locations_system(char out[256]){
 SysEnvRec sys;WDPBRec wd;int e=SysEnvirons(1,&sys);if(e)return e;
 memset(&wd,0,sizeof wd);wd.ioVRefNum=sys.sysVRefNum;e=PBGetWDInfoSync(&wd);if(e)return e;
 long id=wd.ioWDDirID;if(id<=2){HParamBlockRec p;memset(&p,0,sizeof p);p.volumeParam.ioVRefNum=wd.ioWDVRefNum;e=PBHGetVInfoSync(&p);if(e)return e;id=p.volumeParam.ioVFndrInfo[0];}
 return id<=2?dirNFErr:locations_directory(wd.ioWDVRefNum,id,out);
}
#else
#define locations_system LOCATIONS_SYSTEM_PATH
#endif
static int locations_record_path(char out[256]){int e=locations_system(out);if(e)return e;if(strlen(out)+sizeof LOCATION_LEAF>256)return bdNamErr;strcat(out,LOCATION_LEAF);return 0;}
static int locations_read(InstallLocations *p){
 char path[256];int e=locations_record_path(path);if(e)return e;
 short ref;e=update_open(path,0,0,&ref);if(e)return e;unsigned char bytes[LOCATION_BYTES];long n;
 e=GetEOF(ref,&n);if(!e&&n!=sizeof bytes)e=paramErr;
 if(!e){e=FSRead(ref,&n,bytes);if(!e&&n!=sizeof bytes)e=ioErr;}int x=FSClose(ref);if(!e)e=x;
 return e?e:locations_decode(bytes,p);
}
static int locations_default(InstallLocations *p){
 int e=locations_system(p->data);if(e)return e;
 e=recovery_install_path(p->data,p->core);if(e)return e;
 if(strlen(p->data)+13>255)return bdNamErr;strcat(p->data,"Clarice Data:");return 0;
}
/* Missing record retains the historical System Folder convention. A damaged
 * record fails closed: silently selecting a different pseudo-root loses data. */
static int locations_load(InstallLocations *p){int e=locations_read(p);return e==fnfErr?locations_default(p):e;}
static inline int locations_core(const char *root,char out[256]){
 InstallLocations p;int e=locations_read(&p);if(e==fnfErr)return recovery_install_path(root,out);
 if(e)return e;if(strcmp(root,p.data))return paramErr;strcpy(out,p.core);return 0;
}
#ifdef CLARICE_OFFLINE_INSTALLER
static int locations_write(const InstallLocations *p){
 unsigned char bytes[LOCATION_BYTES];int e=locations_encode(p,bytes);if(e)return e;
 InstallLocations old;e=locations_read(&old);if(!e)return strcmp(old.core,p->core)||strcmp(old.data,p->data)?paramErr:0;if(e!=fnfErr)return e;
 char path[256],stage[256];e=locations_record_path(path);if(e)return e;
 if(strlen(path)+4>255)return bdNamErr;strcpy(stage,path);strcat(stage,".new");
 short volume,ref=-1;e=update_volume(stage,&volume);if(e)return e;unsigned char name[256];update_pstr(name,stage);
 CInfoPBRec c;e=update_info(stage,&c);
 if(!e){
  if(c.hFileInfo.ioFlAttrib&17||c.hFileInfo.ioFlRLgLen||c.hFileInfo.ioFlFndrInfo.fdType!='DATA'||c.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC')return paramErr;
  e=update_open(stage,0,0,&ref);long n=0;unsigned char prefix[LOCATION_BYTES];
  if(!e)e=GetEOF(ref,&n);if(!e&&(n<0||n>sizeof prefix))e=paramErr;
  if(!e){long want=n;e=FSRead(ref,&n,prefix);if(!e&&(n!=want||memcmp(prefix,bytes,n)))e=paramErr;}
  if(ref>=0){int x=FSClose(ref);ref=-1;if(!e)e=x;}if(e)return e;
 }else if(e==fnfErr)e=HCreate(volume,0,name,'CLRC','DATA');if(e)return e;
 e=update_open(stage,0,1,&ref);if(!e){long n=sizeof bytes;e=FSWrite(ref,&n,bytes);if(!e&&n!=sizeof bytes)e=ioErr;int x=FSClose(ref);if(!e)e=x;}
 if(!e)e=update_flush(stage);
 if(!e){unsigned char check[LOCATION_BYTES];e=update_open(stage,0,0,&ref);if(!e){long n=sizeof check;e=FSRead(ref,&n,check);if(!e&&(n!=sizeof check||memcmp(check,bytes,sizeof check)))e=paramErr;int x=FSClose(ref);if(!e)e=x;}}
 if(!e)e=update_rename(stage,path);return e?e:update_flush(path);
}
#endif
#endif
