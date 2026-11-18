/* Native HFS open-file guard for a future application updater.
 * Uses documented File Manager queries; errors fail closed. Not a lock or
 * proof of power-loss safety. Caller must recheck immediately before commit. */
#ifndef CLARICE_UPDATE_OPEN_GUARD_H
#define CLARICE_UPDATE_OPEN_GUARD_H
#include <Files.h>
#include <Errors.h>
#include <string.h>
static int update_open_guard(const char *native,int *busy){
 unsigned char name[256];size_t n=strlen(native);short ref=-1;FCBPBRec identity,pb;int e;
 *busy=1;if(!n||n>255||native[0]==':'||!strchr(native,':'))return bdNamErr;
 name[0]=n;memcpy(name+1,native,n);
#ifdef UPDATE_OPEN_RESOURCE
 e=UPDATE_OPEN_RESOURCE(native,&ref);
#else
 e=HOpenRF(0,0,name,fsRdPerm,&ref);
#endif
 if(e)return e;
 memset(&identity,0,sizeof identity);identity.ioRefNum=ref;
 e=PBGetFCBInfoSync(&identity);int close_error=FSClose(ref);if(e||close_error)return e?e:close_error;
 for(int i=1;i<4096;i++){
  memset(&pb,0,sizeof pb);pb.ioFCBIndx=i;pb.ioVRefNum=identity.ioFCBVRefNum;
  e=PBGetFCBInfoSync(&pb);if(e==fnOpnErr){*busy=0;return 0;}if(e)return e;
  if(pb.ioFCBFlNm==identity.ioFCBFlNm)return 0;
 }
 return paramErr;
}
#endif
