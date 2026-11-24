/* Explicit-update/recovery work only. Reuses native HFS and UpdateOps; no
 * startup inventory walk, resident cache or new executable loading facility. */
#ifndef CLARICE_RECOVERY_NATIVE_H
#define CLARICE_RECOVERY_NATIVE_H
#include <Memory.h>
#include "recovery-record.h"
#include "native-files.h"
#include "fork-crc.h"
#include "transaction.h"
#include "stamp.h"
#include "open-guard.h"
#ifndef RECOVERY_ALLOC
#define RECOVERY_ALLOC(n) NewPtr(n)
#define RECOVERY_FREE(p) DisposePtr((Ptr)(p))
#endif
#ifndef RECOVERY_POLL
#define RECOVERY_POLL() 0
#endif
typedef struct {RecoveryHeader header;RecoveryFile *files;} RecoverySnapshot;
static int recovery_join(const char *dir,const char *leaf,char out[256]){
 if(strlen(dir)+strlen(leaf)+2>256)return bdNamErr;strcpy(out,dir);strcat(out,":");strcat(out,leaf);return 0;
}
#ifdef RECOVERY_COMMAND
#define recovery_create cli_native_create
#define recovery_delete cli_native_delete
#define recovery_set cli_native_set
#define recovery_rename cli_native_rename
#define recovery_lock cli_native_lock
#else
#define recovery_rename update_rename
static int recovery_lock(const char *path,int locked){short v;int e=update_volume(path,&v);unsigned char p[256];if(e)return e;update_pstr(p,path);HParamBlockRec pb;memset(&pb,0,sizeof pb);pb.fileParam.ioVRefNum=v;pb.fileParam.ioNamePtr=p;return locked?PBHSetFLockSync(&pb):PBHRstFLockSync(&pb);}
static int recovery_create(const char *path,unsigned long type,unsigned long creator){short v;int e=update_volume(path,&v);unsigned char p[256];if(e)return e;update_pstr(p,path);return HCreate(v,0,p,creator,type);}
static int recovery_delete(const char *path){short v;int e=update_volume(path,&v);unsigned char p[256];if(e)return e;update_pstr(p,path);return HDelete(v,0,p);}
static int recovery_set(const char *path,CInfoPBRec *c){short v;int e=update_volume(path,&v);unsigned char p[256];if(e)return e;update_pstr(p,path);c->hFileInfo.ioNamePtr=p;c->hFileInfo.ioVRefNum=v;c->hFileInfo.ioDirID=0;c->hFileInfo.ioFDirIndex=0;return PBSetCatInfoSync(c);}
#endif
static int recovery_read(short ref,void *p,long n){long got=n;int e=FSRead(ref,&got,p);return e?e:got==n?0:ioErr;}
static int recovery_write(short ref,const void *p,long n){long got=n;int e=FSWrite(ref,&got,p);return e?e:got==n?0:ioErr;}
static int recovery_fork(const char *source,const char *dest,int fork,uint32_t size,uint32_t *crc,int application){
 short in=-1,out=-1;int e=update_open(source,fork,0,&in);if(e)return e;long actual;e=GetEOF(in,&actual);if(!e&&(actual<0||(uint32_t)actual!=size))e=paramErr;
 if(!e&&dest)e=update_open(dest,fork,1,&out);
 unsigned char buf[2048];uint32_t offset=0;UpdateCRC32 hash;update_crc32_init(&hash);
 while(!e&&offset<size){long n=size-offset>sizeof buf?sizeof buf:size-offset;e=recovery_read(in,buf,n);if(!e&&out>=0)e=recovery_write(out,buf,n);
  if(!e){if(application&&fork)update_fork_crc_bytes(&hash,1,offset,buf,n);else update_crc32_bytes(&hash,buf,n);offset+=n;e=RECOVERY_POLL();}}
 if(out>=0){int x=FSClose(out);if(!e)e=x;}int x=FSClose(in);if(!e)e=x;if(!e)*crc=update_crc32_sum(&hash);return e;
}
static int recovery_stat(const char *path,const char *name,RecoveryFile *f){
 CInfoPBRec c;int e=update_info(path,&c);if(e)return e;if(c.hFileInfo.ioFlAttrib&16)return paramErr;
 memset(f,0,sizeof *f);strcpy(f->name,name);f->id=c.hFileInfo.ioDirID;f->data=c.hFileInfo.ioFlLgLen;f->resource=c.hFileInfo.ioFlRLgLen;f->created=c.hFileInfo.ioFlCrDat;f->modified=c.hFileInfo.ioFlMdDat;memcpy(f->finder,&c.hFileInfo.ioFlFndrInfo,16);memcpy(f->extra,&c.hFileInfo.ioFlXFndrInfo,16);return 0;
}
/* Policy is explicit in CLRSNAP2; CLRSNAP1 semantics never change. */
static int recovery_normalized(const RecoveryFile *f){return f->checksum_policy==1||(!f->checksum_policy&&(!strcmp(f->name,"@core")||!strcmp(f->name,"@helper")));}
static int recovery_select_policy(const char *path,RecoveryFile *f){
 f->checksum_policy=2;if(f->resource<256)return 0;
 short ref;int e=update_open(path,1,0,&ref);if(e)return e;unsigned char h[16];e=recovery_read(ref,h,sizeof h);int x=FSClose(ref);if(!e)e=x;
 if(!e&&!update_resource_layout(h,f->resource))f->checksum_policy=1;return e;
}
/* Finder may assign an icon position/folder and set hasBeenInited merely by
 * opening the snapshot folder, including FXInfo.fdIconID (its first word).
 * These are display state, not file integrity.
 * The saved full metadata is still applied on restoration. */
static int recovery_finder_equal(const unsigned char *a,const unsigned char *b){
 return !memcmp(a,b,8)&&((a[8]^b[8])&0xfe)==0&&a[9]==b[9];
}
static int recovery_check_file(const char *path,const RecoveryFile *f,int identity){
 RecoveryFile c;int e=recovery_stat(path,f->name,&c);if(e)return e;
 if((identity&&f->id!=c.id)||f->data!=c.data||f->resource!=c.resource||f->created!=c.created||f->modified!=c.modified||!recovery_finder_equal(f->finder,c.finder)||memcmp(f->extra+2,c.extra+2,14))return paramErr;
 if(f->checksum_policy==1){e=recovery_select_policy(path,&c);if(e)return e;if(c.checksum_policy!=1)return paramErr;}
 uint32_t hash;int app=recovery_normalized(f);
 e=recovery_fork(path,0,0,f->data,&hash,app);if(!e&&hash!=f->data_crc)e=paramErr;
 if(!e)e=recovery_fork(path,0,1,f->resource,&hash,app);if(!e&&hash!=f->resource_crc)e=paramErr;return e;
}
/* Destination must be absent. Close both forks and copy all Finder metadata
 * before publishing this file in a completed recovery inventory. */
static int recovery_copy(const char *source,const char *dest,const char *name,RecoveryFile *f){
 CInfoPBRec c;int e=update_info(source,&c);if(e)return e;if(c.hFileInfo.ioFlAttrib&16)return permErr;
 e=recovery_stat(source,name,f);if(!e)e=recovery_select_policy(source,f);if(e)return e;f->source_id=f->id;
 e=recovery_create(dest,c.hFileInfo.ioFlFndrInfo.fdType,c.hFileInfo.ioFlFndrInfo.fdCreator);if(e)return e;
 int app=recovery_normalized(f);
 e=recovery_fork(source,dest,0,f->data,&f->data_crc,app);if(!e)e=recovery_fork(source,dest,1,f->resource,&f->resource_crc,app);
 if(!e)e=recovery_set(dest,&c);if(!e){RecoveryFile target;e=recovery_stat(dest,name,&target);if(!e)f->id=target.id;}
 if(!e)e=recovery_check_file(dest,f,1);
 if(e){int cleanup=recovery_delete(dest);if(cleanup)return cleanup;}return e;
}
/* A reset can leave a prefix of the copy/plan before any installed rename.
 * Resume only byte-for-byte prefixes of this verified snapshot/encoded plan;
 * never truncate, delete, or adopt a different file at a reserved name. */
static int recovery_prefix(const char *source,const char *dest,int fork,uint32_t limit,int application){
 short a=-1,b=-1;int e=update_open(dest,fork,0,&b);if(e)return e;long size;e=GetEOF(b,&size);
 if(!e&&(size<0||(uint32_t)size>limit))e=paramErr;if(!e)e=update_open(source,fork,0,&a);
 unsigned char x[512],y[512];uint32_t offset=0;while(!e&&size){long n=size>(long)sizeof x?(long)sizeof x:size;e=recovery_read(a,x,n);if(!e)e=recovery_read(b,y,n);if(!e){for(long j=0;j<n;j++)if(!(application&&fork&&offset+j>=16&&offset+j<128)&&x[j]!=y[j]){e=paramErr;break;}}size-=n;offset+=n;}
 if(a>=0){int z=FSClose(a);if(!e)e=z;}int z=FSClose(b);return e?e:z;
}
static int recovery_resume_copy(const char *source,const char *dest,const RecoveryFile *f){
 CInfoPBRec c,original;int e=update_info(dest,&c);if(e==fnfErr){RecoveryFile copy;return recovery_copy(source,dest,f->name,&copy);}if(e)return e;
 if(c.hFileInfo.ioFlAttrib&16)return permErr;
 if(c.hFileInfo.ioFlAttrib&1)return recovery_check_file(dest,f,0);
 e=update_info(source,&original);if(e)return e;
 if(c.hFileInfo.ioFlFndrInfo.fdType!=original.hFileInfo.ioFlFndrInfo.fdType||c.hFileInfo.ioFlFndrInfo.fdCreator!=original.hFileInfo.ioFlFndrInfo.fdCreator)return paramErr;
 int app=recovery_normalized(f);
 e=recovery_prefix(source,dest,0,f->data,app);if(!e)e=recovery_prefix(source,dest,1,f->resource,app);if(e)return e;
 uint32_t crc;e=recovery_fork(source,dest,0,f->data,&crc,app);if(!e&&crc!=f->data_crc)e=paramErr;
 if(!e)e=recovery_fork(source,dest,1,f->resource,&crc,app);if(!e&&crc!=f->resource_crc)e=paramErr;
 if(!e)e=recovery_set(dest,&original);return e?e:recovery_check_file(dest,f,0);
}
static void recovery_dispose(RecoverySnapshot *s){if(s->files)RECOVERY_FREE(s->files);memset(s,0,sizeof *s);}
static int recovery_prefix_bytes(short ref,const unsigned char *bytes,long n,long *left){
 unsigned char block[512];if(n>*left)n=*left;while(n){long take=n>(long)sizeof block?(long)sizeof block:n;int e=recovery_read(ref,block,take);if(e)return e;if(memcmp(block,bytes,take))return paramErr;bytes+=take;n-=take;*left-=take;}return 0;
}
static int recovery_inventory_prefix(const char *path,const RecoverySnapshot *s){
 short ref;int e=update_open(path,0,0,&ref);if(e)return e;long left;e=GetEOF(ref,&left);
 if(!e&&(left<0||(uint32_t)left>RECOVERY_HEADER+s->header.count*RECOVERY_ENTRY+4))e=paramErr;
 unsigned char b[RECOVERY_HEADER];UpdateCRC32 hash;update_crc32_init(&hash);
 if(!e)e=recovery_header_encode(&s->header,b);if(!e){update_crc32_bytes(&hash,b,RECOVERY_HEADER);e=recovery_prefix_bytes(ref,b,RECOVERY_HEADER,&left);}
 for(uint32_t i=0;!e&&i<s->header.count;i++){e=recovery_file_encode(&s->files[i],b);if(!e&&!s->header.format&&s->files[i].checksum_policy)e=paramErr;
  if(!e){update_crc32_bytes(&hash,b,RECOVERY_ENTRY);e=recovery_prefix_bytes(ref,b,RECOVERY_ENTRY,&left);}}
 if(!e){recovery_put32(b,update_crc32_sum(&hash));e=recovery_prefix_bytes(ref,b,4,&left);}int x=FSClose(ref);return e?e:x;
}
static int recovery_inventory(const char *dir,RecoverySnapshot *s,int write){
 char path[256];int e=recovery_join(dir,RECOVERY_MANIFEST,path);if(e)return e;short ref=-1;unsigned char b[RECOVERY_HEADER];UpdateCRC32 hash;update_crc32_init(&hash);
 if(write){e=recovery_header_encode(&s->header,b);if(e)return e;e=recovery_create(path,'DATA','CLRC');if(e==dupFNErr)e=recovery_inventory_prefix(path,s);if(e)return e;e=update_open(path,0,1,&ref);if(!e)e=recovery_write(ref,b,RECOVERY_HEADER);}
 else{memset(s,0,sizeof *s);e=update_open(path,0,0,&ref);if(!e)e=recovery_read(ref,b,RECOVERY_HEADER);if(!e)e=recovery_header_decode(b,&s->header);
  long size;if(!e)e=GetEOF(ref,&size);if(!e&&(uint32_t)size!=RECOVERY_HEADER+s->header.count*RECOVERY_ENTRY+4)e=paramErr;
  if(!e){s->files=(RecoveryFile*)RECOVERY_ALLOC(s->header.count*sizeof *s->files);if(!s->files)e=memFullErr;}}
 if(!e)update_crc32_bytes(&hash,b,RECOVERY_HEADER);
 for(uint32_t i=0;!e&&i<s->header.count;i++){
  if(write){e=recovery_file_encode(&s->files[i],b);if(!e)e=recovery_write(ref,b,RECOVERY_ENTRY);}
  else{e=recovery_read(ref,b,RECOVERY_ENTRY);if(!e)e=recovery_file_decode(b,&s->files[i]);}
  if(!e&&!s->header.format&&s->files[i].checksum_policy)e=paramErr;
  if(!e){update_crc32_bytes(&hash,b,RECOVERY_ENTRY);for(uint32_t j=0;j<i;j++)if(!strcmp(s->files[j].name,s->files[i].name)){e=paramErr;break;}}
 }
 if(!e){uint32_t crc=update_crc32_sum(&hash);if(write){recovery_put32(b,crc);e=recovery_write(ref,b,4);}else{e=recovery_read(ref,b,4);if(!e&&update_be32(b)!=crc)e=paramErr;}}
 if(ref>=0){int x=FSClose(ref);if(!e)e=x;}
 if(!e&&write)e=update_flush(path);if(e&&!write)recovery_dispose(s);return e;
}
static int recovery_verify(const char *dir,RecoverySnapshot *s){
 char expected[256];const char *names[]={RECOVERY_CURRENT,RECOVERY_PENDING,RECOVERY_RETIRED};int bound=0;
 if(strlen(s->header.root)+12+strlen(RECOVERY_PENDING)+1>sizeof expected)return bdNamErr;
 for(int n=0;n<3;n++){strcpy(expected,s->header.root);strcat(expected,"etc:clarice:");strcat(expected,names[n]);if(!strcmp(dir,expected))bound=1;}if(!bound)return paramErr;
 int core=0,helper=0,catalog=0;for(uint32_t i=0;i<s->header.count;i++){
  char path[256];RecoveryFile *f=&s->files[i];catalog+=!strcmp(f->name,"@catalog");if(!f->id)continue;
  int e=recovery_member(dir,f->name,path);if(!e)e=recovery_check_file(path,f,1);if(e)return e;
  if(!strcmp(f->name,"@core")){uint32_t build;unsigned system;e=update_stamp(path,&build,&system);if(e)return e;if(build!=s->header.build)return paramErr;core++;}
  helper+=!strcmp(f->name,"@helper");
 }
 return core==1&&helper==1&&catalog==1?0:paramErr;
}
/* Exact former file identities bind repository backup cleanup. A same-named
 * historical/manual file is never sufficient authority to remove it. */
static int recovery_clean_tools(const RecoverySnapshot *s){
 for(uint32_t i=0;i<s->header.count;i++){
  const RecoveryFile *f=&s->files[i];if(f->name[0]=='@'||!f->source_id)continue;
  char path[256];if(strlen(s->header.root)+strlen(f->name)+16>sizeof path)return bdNamErr;
  strcpy(path,s->header.root);strcat(path,"bin:.repo-");strcat(path,f->name);strcat(path,".old");
  CInfoPBRec c;int e=update_info(path,&c);if(e==fnfErr)continue;if(e)return e;
  if((uint32_t)c.hFileInfo.ioDirID!=f->source_id)return paramErr;e=recovery_check_file(path,f,0);if(e)return e;e=recovery_delete(path);if(e)return e;
 }return update_flush(s->header.root);
}
/* Explicit command rename only, during successful update completion. The
 * verified whole-release snapshot remains the recovery owner. Never infer
 * ownership from a filename alone, or remove a modified/user-replaced tool. */
static int recovery_retire_tool(const RecoverySnapshot *s,const char *oldname,const char *newname,int (*validate)(const char *)){
 const RecoveryFile *f=0;for(uint32_t i=0;i<s->header.count;i++)if(!strcmp(s->files[i].name,oldname)){f=&s->files[i];break;}
 if(!f||!f->id||!f->source_id)return 0;
 char oldpath[256],newpath[256];if(strlen(s->header.root)+strlen(oldname)+5>256||strlen(s->header.root)+strlen(newname)+5>256)return bdNamErr;
 strcpy(oldpath,s->header.root);strcat(oldpath,"bin:");strcat(oldpath,oldname);
 strcpy(newpath,s->header.root);strcat(newpath,"bin:");strcat(newpath,newname);
 CInfoPBRec c;int e=update_info(oldpath,&c);if(e==fnfErr)return 0;if(e)return e;
 if((uint32_t)c.hFileInfo.ioDirID!=f->source_id)return paramErr;
 e=recovery_check_file(oldpath,f,0);if(!e&&validate)e=validate(newpath);if(e)return e;
 int busy=1;e=update_open_guard(oldpath,&busy);if(!e&&busy)e=fBsyErr;
 if(!e)e=recovery_delete(oldpath);return e?e:update_flush(oldpath);
}
/* Used only after another complete snapshot has been validated. Unlisted files
 * prevent directory deletion; listed files must still have their recorded IDs.
 * Missing members are accepted so interruption during pruning is resumable. */
static int recovery_prune(const char *dir,uint32_t expected_id){
 CInfoPBRec directory;int e=update_info(dir,&directory);if(e)return e;if(!expected_id||(uint32_t)directory.hFileInfo.ioDirID!=expected_id||!(directory.hFileInfo.ioFlAttrib&16))return paramErr;
 RecoverySnapshot s;e=recovery_inventory(dir,&s,0);
 if(e==fnfErr){/* Only an empty, identity-bound directory can be removed. */e=recovery_delete(dir);return e?e:update_flush(dir);}if(e)return e;
 for(uint32_t i=0;!e&&i<s.header.count;i++){if(!s.files[i].id)continue;char path[256];CInfoPBRec c;e=recovery_member(dir,s.files[i].name,path);if(!e)e=update_info(path,&c);if(e==fnfErr){e=0;continue;}
  if(!e&&(uint32_t)c.hFileInfo.ioDirID!=s.files[i].id)e=paramErr;
  if(!e&&(c.hFileInfo.ioFlAttrib&1))e=recovery_lock(path,0);
  if(!e)e=recovery_delete(path);
 }
 recovery_dispose(&s);if(e)return e;
 char path[256];e=recovery_join(dir,RECOVERY_MANIFEST,path);if(!e)e=recovery_delete(path);if(!e)e=recovery_delete(dir);return e?e:update_flush(dir);
}
/* Snapshot rotation is the same three-slot native replacement used by Core.
 * The old generation is pruned only after the new generation passes readback. */
typedef struct {char path[3][256];} RecoveryRotation;
static int recovery_rotation_probe(void *v,int n){RecoveryRotation *r=v;CInfoPBRec c;int e=update_info(r->path[n],&c);return e==fnfErr?0:e?e:(c.hFileInfo.ioFlAttrib&16)?1:paramErr;}
static int recovery_rotation_check(void *v,int n){RecoveryRotation *r=v;RecoverySnapshot s;int e=recovery_inventory(r->path[n],&s,0);if(!e){e=recovery_verify(r->path[n],&s);recovery_dispose(&s);}return e;}
static int recovery_rotation_closed(void *v,int n){(void)v;(void)n;return 0;}
static int recovery_rotation_move(void *v,int a,int b){RecoveryRotation *r=v;return recovery_rename(r->path[a],r->path[b]);}
static int recovery_rotation_flush(void *v){return update_flush(((RecoveryRotation*)v)->path[0]);}
static int recovery_prune_retired(RecoveryRotation *r){
 RecoverySnapshot current;int e=recovery_inventory(r->path[0],&current,0);if(e)return e;e=recovery_verify(r->path[0],&current);uint32_t id=current.header.retired_id;recovery_dispose(&current);return e?e:recovery_prune(r->path[2],id);
}
static int recovery_rotate(RecoveryRotation *r){
 int a=recovery_rotation_probe(r,0),b=recovery_rotation_probe(r,2),p=recovery_rotation_probe(r,1),e;if(a<0||b<0||p<0)return a<0?a:b<0?b:p;
 if(b){if(a){if((e=recovery_rotation_check(r,0)))return e;if((e=recovery_prune_retired(r)))return e;}else{if((e=recovery_rotation_check(r,2)))return e;if((e=recovery_rotation_move(r,2,0))||(e=recovery_rotation_flush(r)))return e;}}
 if(!p)return recovery_rotation_check(r,0);
 UpdateOps ops={r,recovery_rotation_probe,recovery_rotation_check,recovery_rotation_closed,recovery_rotation_move,recovery_rotation_flush};UpdateResult result=update_commit(&ops);if(result.error)return result.error;
 return result.previous_retained?recovery_prune_retired(r):0;
}

/* An interrupted pre-upgrade preparation can hold the previous helper.
 * Replace only a verified duplicate of the intact previous snapshot's helper;
 * never discard an unknown file or the sole recovery copy. */
static int recovery_refresh_helper(RecoveryRotation *rotation,const char *dest,const char *source,RecoveryFile *file,int legacy){
 RecoverySnapshot old;int e=recovery_inventory(rotation->path[0],&old,0);
 if((e==fnfErr||e==dirNFErr)&&!legacy){
  /* A first-ever pre-inventory attempt has no verified old helper to compare.
     Preserve the unknown residue, as Core already does for orphaned stages;
     never delete it or adopt it into a known-good snapshot. */
  CInfoPBRec c;int x=update_info(rotation->path[0],&c);if(x!=fnfErr)return x?x:e;
  char inventory[256];x=recovery_join(rotation->path[1],RECOVERY_MANIFEST,inventory);if(x)return x;
  x=update_info(inventory,&c);if(x!=fnfErr)return x?x:paramErr;
  x=update_info(dest,&c);if(x)return x;if(c.hFileInfo.ioFlAttrib&17)return permErr;
  char saved[256];strcpy(saved,rotation->path[1]);char *leaf=strrchr(saved,':');if(!leaf)return paramErr;
  int n=snprintf(leaf+1,sizeof saved-(leaf+1-saved),"Recovery Helper-%08lx",(unsigned long)c.hFileInfo.ioDirID);
  if(n<0||(unsigned)n>=sizeof saved-(leaf+1-saved))return bdNamErr;
  int busy=1;x=update_open_guard(dest,&busy);if(!x&&busy)x=fBsyErr;
  /* HRename cannot move across directories. Preserve through the existing
     bounded, resumable fork copy and verify before deleting the residue. */
  RecoveryFile residue;if(!x)x=recovery_stat(dest,"@helper",&residue);if(!x)x=recovery_select_policy(dest,&residue);
  if(!x)x=recovery_fork(dest,0,0,residue.data,&residue.data_crc,recovery_normalized(&residue));
  if(!x)x=recovery_fork(dest,0,1,residue.resource,&residue.resource_crc,recovery_normalized(&residue));
  if(!x)x=recovery_resume_copy(dest,saved,&residue);if(!x)x=update_flush(saved);
  if(!x)x=recovery_check_file(dest,&residue,1);if(!x)x=recovery_delete(dest);if(!x)x=update_flush(dest);
  return x?x:recovery_copy(source,dest,"@helper",file);
 }
 if(e)return e;
 e=recovery_verify(rotation->path[0],&old);const RecoveryFile *helper=0;
 for(uint32_t i=0;!e&&i<old.header.count;i++)if(!strcmp(old.files[i].name,"@helper"))helper=&old.files[i];
 if(!e){
  if(!helper)e=paramErr;
  else{RecoveryFile duplicate;e=recovery_stat(dest,"@helper",&duplicate);
   /* Identical cached helpers may have been created on different dates.
    * Only this disposable duplicate comparison permits those dates to differ. */
   if(!e){RecoveryFile expected=*helper;expected.created=duplicate.created;expected.modified=duplicate.modified;e=recovery_check_file(dest,&expected,0);}
  }
 }
 if(!e&&legacy)*file=*helper;
 recovery_dispose(&old);if(e||legacy)return e;
 e=recovery_delete(dest);if(!e)e=update_flush(dest);
 return e?e:recovery_copy(source,dest,"@helper",file);
}
/* Caller supplies one live installation's inventory, never a selection of old
   backup files. All entries are copied before the generation becomes current. */
static int recovery_prepare(RecoveryRotation *rotation,RecoverySnapshot *s,const char *helper,int (*validate)(const char *)){
 /* Tool names are lowercase ASCII. These native leaves collide on HFS;
    reject before any snapshot writes, without changing legacy decoding. */
 for(uint32_t i=0;i<s->header.count;i++)if(!strcmp(s->files[i].name,"core")||!strcmp(s->files[i].name,"catalog")||!strcmp(s->files[i].name,"inventory"))return paramErr;
 s->header.format=2;int e=recovery_rotation_probe(rotation,1);if(e<0)return e;int exists=e;
 if(exists){RecoverySnapshot pending;e=recovery_inventory(rotation->path[1],&pending,0);
  if(!e){if(strcmp(pending.header.root,s->header.root)||strcmp(pending.header.origin,s->header.origin)||pending.header.build!=s->header.build||pending.header.origin_id!=s->header.origin_id||pending.header.origin_modified!=s->header.origin_modified)e=paramErr;
   if(!e)e=recovery_verify(rotation->path[1],&pending);recovery_dispose(&pending);return e?e:recovery_rotate(rotation);}
  if(e!=fnfErr&&e!=eofErr&&e!=paramErr)return e;
  char inventory[256];short ref;int x=recovery_join(rotation->path[1],RECOVERY_MANIFEST,inventory);if(x)return x;
  x=update_open(inventory,0,0,&ref);if(x&&x!=fnfErr)return x;
  if(!x){unsigned char magic[8];int read=recovery_read(ref,magic,8);int closed=FSClose(ref);if(closed)return closed;
   if(read&&read!=eofErr)return read;if(!read&&!memcmp(magic,"CLRSNAP1",8))s->header.format=0;
  }
 }
 CInfoPBRec retired;e=update_info(rotation->path[0],&retired);if(!e)s->header.retired_id=retired.hFileInfo.ioDirID;else if(e!=fnfErr)return e;
 e=0; /* No prior generation is normal, including a partial first-copy retry. */
 if(!exists){short volume;e=update_volume(rotation->path[1],&volume);if(e)return e;unsigned char pn[256];update_pstr(pn,rotation->path[1]);
#ifdef RECOVERY_COMMAND
  e=cli_native_mkdir(rotation->path[1]);
#else
  HParamBlockRec pb;memset(&pb,0,sizeof pb);pb.fileParam.ioVRefNum=volume;pb.fileParam.ioNamePtr=pn;e=PBDirCreateSync(&pb);
#endif
  if(e)return e;
 }
 for(uint32_t i=0;!e&&i<s->header.count;i++){
  char source[256],dest[256];RecoveryFile *f=&s->files[i];
  if(!strcmp(f->name,"@core"))strcpy(source,s->header.origin);
  else if(!strcmp(f->name,"@helper"))strcpy(source,helper);
  else{const char *leaf=!strcmp(f->name,"@catalog")?"etc:clarice:installed.catalog":"bin:";
   if(strlen(s->header.root)+strlen(leaf)+strlen(f->name)+1>256){e=bdNamErr;break;}
   strcpy(source,s->header.root);strcat(source,leaf);if(strcmp(f->name,"@catalog"))strcat(source,f->name);
  }
  CInfoPBRec c;e=update_info(source,&c);if(e==fnfErr&&f->name[0]!='@'){e=0;f->id=0;continue;}
  if(e==fnfErr&&!strcmp(f->name,"@catalog")){e=0;f->id=0;continue;}
  if(!e&&f->name[0]!='@'&&validate)e=validate(source);
  if(!e)e=recovery_member(rotation->path[1],f->name,dest);
  if(!e){char name[24];strcpy(name,f->name);CInfoPBRec existing;int found=update_info(dest,&existing);
   if(found==fnfErr){e=recovery_copy(source,dest,name,f);if(!e&&!s->header.format){f->checksum_policy=0;e=recovery_fork(source,0,1,f->resource,&f->resource_crc,recovery_normalized(f));if(!e)e=recovery_check_file(dest,f,1);}}
   else if(found)e=found;
   else{e=recovery_stat(source,name,f);f->source_id=f->id;if(!e&&s->header.format)e=recovery_select_policy(source,f);int app=recovery_normalized(f);
    if(!e)e=recovery_fork(source,0,0,f->data,&f->data_crc,app);if(!e)e=recovery_fork(source,0,1,f->resource,&f->resource_crc,app);
    if(!e)e=recovery_resume_copy(source,dest,f);
    if(e==paramErr&&!strcmp(name,"@helper")){e=recovery_refresh_helper(rotation,dest,source,f,!s->header.format);if(!e)e=update_info(dest,&existing);}
    if(!e)f->id=existing.hFileInfo.ioDirID;
   }
  }
  /* Carry the lock on the native snapshot copy; never unlock the source. */
  if(!e&&(c.hFileInfo.ioFlAttrib&1))e=recovery_lock(dest,1);
 }
 if(!e)e=recovery_inventory(rotation->path[1],s,1);
 if(!e)e=recovery_verify(rotation->path[1],s);
 if(!e)return recovery_rotate(rotation);
 /* No active files have changed. Keep verified/prefix copies for retry;
    an incompatible or non-prefix file is retained and stops preparation. */
 return e;
}
#endif
