/* Resolve the actual running application via its open resource-file identity.
 * Never guess its name, launch directory or boot volume. */
#ifndef CLARICE_UPDATE_IDENTITY_H
#define CLARICE_UPDATE_IDENTITY_H
#include <Files.h>
#include <SegLoad.h>
#include <Errors.h>
#include <string.h>
static int update_application_path(char out[256]){
 unsigned char name[256];short ref;Handle parms;FCBPBRec f;CInfoPBRec cat;HParamBlockRec v;
 GetAppParms(name,&ref,&parms);memset(&f,0,sizeof f);f.ioRefNum=ref;f.ioNamePtr=name;
 int e=PBGetFCBInfoSync(&f);if(e)return e;
 char tail[256],part[256];memcpy(tail,name+1,name[0]);tail[name[0]]=0;long dir=f.ioFCBParID;
 for(int depth=0;dir>2&&depth<32;depth++){
  memset(&cat,0,sizeof cat);cat.dirInfo.ioNamePtr=name;cat.dirInfo.ioVRefNum=f.ioFCBVRefNum;cat.dirInfo.ioDrDirID=dir;cat.dirInfo.ioFDirIndex=-1;
  e=PBGetCatInfoSync(&cat);if(e)return e;
  if(name[0]+strlen(tail)+2>sizeof part)return bdNamErr;
  memcpy(part,name+1,name[0]);part[name[0]]=':';strcpy(part+name[0]+1,tail);strcpy(tail,part);dir=cat.dirInfo.ioDrParID;
 }
 if(dir!=2)return dirNFErr;
 memset(&v,0,sizeof v);v.volumeParam.ioVRefNum=f.ioFCBVRefNum;v.volumeParam.ioNamePtr=name;
 e=PBHGetVInfoSync(&v);if(e)return e;
 if(name[0]+strlen(tail)+2>256)return bdNamErr;
 memcpy(out,name+1,name[0]);out[name[0]]=':';strcpy(out+name[0]+1,tail);return 0;
}
#endif
