#ifndef CLARICE_UPDATE_VERIFY_H
#define CLARICE_UPDATE_VERIFY_H
#include <stdint.h>
#include "fork-crc.h"
#include "native-files.h"
#include "plan.h"
#include "stamp.h"
#ifndef UPDATE_VERIFY_REPORT
#define UPDATE_VERIFY_REPORT(phase,want,have) ((void)0)
#endif
#ifndef UPDATE_VERIFY_POLL
#define UPDATE_VERIFY_POLL() 0
#endif
static int update_hash_fork(const char *path,int fork,uint32_t *hash,uint32_t expected){
 short ref=-1;int e=update_open(path,fork,0,&ref);if(e)return e;long size;e=GetEOF(ref,&size);if(!e&&(size<0||(uint32_t)size!=expected))e=paramErr;
 UpdateCRC32 ctx;update_crc32_init(&ctx);unsigned char buf[2048];uint32_t left=expected;
 while(!e&&left){long n=left>sizeof buf?sizeof buf:left,want=n;e=FSRead(ref,&n,buf);if(!e&&n!=want)e=ioErr;if(!e){if(fork&&left==expected&&(n<16||update_resource_layout(buf,expected)))e=paramErr;else {update_fork_crc_bytes(&ctx,fork,expected-left,buf,n);left-=n;e=UPDATE_VERIFY_POLL();}}}
 int close_error=FSClose(ref);if(!e)e=close_error;if(!e)*hash=update_crc32_sum(&ctx);return e;
}
static int update_verify_structure(const char *path,const UpdatePlan *p){
 CInfoPBRec cat;int e=update_info(path,&cat);if(e)return e;
 if(cat.hFileInfo.ioFlAttrib&17||cat.hFileInfo.ioFlFndrInfo.fdType!='APPL'||cat.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC'||memcmp(&cat.hFileInfo.ioFlFndrInfo,p->metadata,16)||memcmp(&cat.hFileInfo.ioFlXFndrInfo,p->extra,16)||cat.hFileInfo.ioFlCrDat!=p->created||cat.hFileInfo.ioFlMdDat!=p->modified)return paramErr;
 uint32_t build=0;unsigned system=0;e=update_stamp(path,&build,&system);if(e)return e;SysEnvRec env;e=SysEnvirons(1,&env);if(e)return e;
 if(build!=p->build||system>(unsigned short)env.systemVersion)return paramErr;
 return cat.hFileInfo.ioFlLgLen!=p->data_size||cat.hFileInfo.ioFlRLgLen!=p->resource_size?paramErr:0;
}
static int update_verify_candidate(const char *path,const UpdatePlan *p){
 int e=update_verify_structure(path,p);if(e)return e;uint32_t hash;
 e=update_hash_fork(path,0,&hash,p->data_size);if(e)return e;if(hash!=p->data_crc){UPDATE_VERIFY_REPORT("data fork CRC",p->data_crc,hash);return paramErr;}
 e=update_hash_fork(path,1,&hash,p->resource_size);if(e)return e;if(hash!=p->resource_crc){UPDATE_VERIFY_REPORT("resource fork CRC",p->resource_crc,hash);return paramErr;}return 0;
}
#endif
