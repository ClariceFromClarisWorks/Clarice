/* Explicit independent rollback. A snapshot remains immutable throughout.
 * Each file uses UpdateOps and its existing durable plan encoding; repeating
 * recovery skips verified restored files and resumes the outstanding rename. */
#ifndef CLARICE_RECOVERY_RESTORE_H
#define CLARICE_RECOVERY_RESTORE_H
#include "recovery-native.h"
#include "open-guard.h"
#include "plan.h"
#define RECOVERY_RESTORE_PLAN "Restore.plan"
typedef struct {UpdatePlan plan;RecoveryFile file;} RecoveryRestore;
static const char *recovery_restore_slot(RecoveryRestore *r,int slot){return slot==0?r->plan.target:slot==1?r->plan.stage:r->plan.backup;}
static int recovery_restore_probe(void *v,int n){RecoveryRestore *r=v;CInfoPBRec c;int e=update_info(recovery_restore_slot(r,n),&c);if(e)return e==fnfErr?0:e;
 if(c.hFileInfo.ioFlAttrib&17)return permErr;
 if(n==0&&((uint32_t)c.hFileInfo.ioDirID!=r->plan.original_id||c.hFileInfo.ioFlMdDat!=r->plan.original_modified))return paramErr;return 1;}
static int recovery_restore_check(void *v,int n){RecoveryRestore *r=v;return recovery_check_file(recovery_restore_slot(r,n),&r->file,0);}
static int recovery_restore_closed(void *v,int n){int busy=1;int e=update_open_guard(recovery_restore_slot(v,n),&busy);return e?e:busy?fBsyErr:0;}
static int recovery_restore_move(void *v,int a,int b){return update_rename(recovery_restore_slot(v,a),recovery_restore_slot(v,b));}
static int recovery_restore_flush(void *v){return update_flush(((RecoveryRestore*)v)->plan.target);}
static int recovery_plan_read(const char *path,UpdatePlan *p){
 short ref;int e=update_open(path,0,0,&ref);if(e)return e;long size;e=GetEOF(ref,&size);unsigned char b[UPDATE_PLAN_SIZE];if(!e&&size!=sizeof b)e=paramErr;if(!e)e=recovery_read(ref,b,sizeof b);int x=FSClose(ref);if(!e)e=x;return e?e:update_plan_decode(b,p);
}
static int recovery_plan_write(const char *path,const UpdatePlan *p){
 unsigned char b[UPDATE_PLAN_SIZE];int e=update_plan_encode(p,b);if(e)return e;e=recovery_create(path,'DATA','CLRC');if(e)return e;short ref;e=update_open(path,0,1,&ref);if(!e){e=recovery_write(ref,b,sizeof b);int x=FSClose(ref);if(!e)e=x;}if(e){/* No installed rename has begun; remove only this newly created plan. */recovery_delete(path);return e;}return update_flush(path);
}
static int recovery_resume_plan(const char *path,const UpdatePlan *plan){
 unsigned char expected[UPDATE_PLAN_SIZE],actual[UPDATE_PLAN_SIZE];int e=update_plan_encode(plan,expected);if(e)return e;
 CInfoPBRec c;e=update_info(path,&c);if(e)return e;if(c.hFileInfo.ioFlAttrib&17||c.hFileInfo.ioFlFndrInfo.fdType!='DATA'||c.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC'||c.hFileInfo.ioFlRLgLen)return paramErr;
 short ref;e=update_open(path,0,0,&ref);if(e)return e;long n;e=GetEOF(ref,&n);if(!e&&(n<0||n>UPDATE_PLAN_SIZE))e=paramErr;if(!e)e=recovery_read(ref,actual,n);if(!e&&memcmp(actual,expected,n))e=paramErr;int x=FSClose(ref);if(!e)e=x;if(e)return e;
 e=update_open(path,0,1,&ref);if(e)return e;e=recovery_write(ref,expected,sizeof expected);x=FSClose(ref);if(!e)e=x;return e?e:update_flush(path);
}
static int recovery_target(const RecoveryHeader *h,const char *name,char target[256]){
 if(!strcmp(name,"@core"))return recovery_core_target(h,target);
 const char *middle=!strcmp(name,"@catalog")?"etc:clarice:installed.catalog":"bin:";
 if(strlen(h->root)+strlen(middle)+strlen(name)+1>256)return bdNamErr;
 strcpy(target,h->root);strcat(target,middle);if(strcmp(name,"@catalog"))strcat(target,name);return 0;
}
static int recovery_restore_absent(const RecoveryHeader *h,const RecoveryFile *f){
 char target[256];int e=recovery_target(h,f->name,target);if(e)return e;CInfoPBRec c;e=update_info(target,&c);if(e==fnfErr)return 0;if(e)return e;
 if(c.hFileInfo.ioFlAttrib&17||c.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC')return paramErr;
 if(!strcmp(f->name,"@catalog")){
  short ref;e=update_open(target,0,0,&ref);if(e)return e;char header[sizeof("CLARICE-INSTALLED 1\n")-1];e=recovery_read(ref,header,sizeof header);int x=FSClose(ref);if(!e)e=x;if(!e&&memcmp(header,"CLARICE-INSTALLED 1\n",sizeof header))e=paramErr;
 }else{uint32_t crc;if(c.hFileInfo.ioFlFndrInfo.fdType!='CLX1'||(uint32_t)c.hFileInfo.ioFlLgLen!=f->data||c.hFileInfo.ioFlRLgLen)return paramErr;e=recovery_fork(target,0,0,f->data,&crc,0);if(!e&&crc!=f->data_crc)e=paramErr;}
 if(!e){int busy=1;e=update_open_guard(target,&busy);if(!e&&busy)e=fBsyErr;}
 if(!e)e=recovery_delete(target);return e?e:update_flush(target);
}
static int recovery_restore_one(const char *dir,const RecoveryHeader *h,const RecoveryFile *file){
 if(!file->id)return recovery_restore_absent(h,file);
 RecoveryRestore r;memset(&r,0,sizeof r);r.file=*file;char source[256],journal[256],target[256];
 int e=recovery_target(h,file->name,target);if(!e)e=recovery_member(dir,file->name,source);if(!e)e=recovery_join(dir,RECOVERY_RESTORE_PLAN,journal);if(e)return e;
 CInfoPBRec source_info;e=update_info(source,&source_info);if(e)return e;int locked=source_info.hFileInfo.ioFlAttrib&1;
 /* Authenticate a legacy source with its original exact checksum first.
  * Only the transient native-copy expectation may then ignore valid resource
  * scratch. Inventory and existing restore-plan CRCs remain unchanged. */
 if(!file->checksum_policy&&!recovery_normalized(file)&&file->resource>=256){
  e=recovery_check_file(source,file,1);if(e)return e;
  e=recovery_select_policy(source,&r.file);if(e)return e;
  if(r.file.checksum_policy==1){e=recovery_fork(source,0,1,file->resource,&r.file.resource_crc,1);if(e)return e;}
 }
 e=recovery_plan_read(journal,&r.plan);int pending=!e;
 int partial=e==paramErr||e==eofErr;
 if(e&&e!=fnfErr&&!partial)return e;
 if(pending){
  if(!r.plan.kind||strcmp(r.plan.target,target))return paramErr;
  if(r.plan.build!=h->build||r.plan.data_size!=file->data||r.plan.resource_size!=file->resource||r.plan.data_crc!=file->data_crc||r.plan.resource_crc!=file->resource_crc||memcmp(r.plan.metadata,file->finder,16)||memcmp(r.plan.extra,file->extra,16)||r.plan.created!=file->created||r.plan.modified!=file->modified)return paramErr;
 }else{
  e=recovery_check_file(target,&r.file,0);if(!e)return locked?recovery_lock(target,1):0;
  CInfoPBRec c;e=update_info(target,&c);if(e&&e!=fnfErr)return e;
  r.plan.kind=e==fnfErr?2:1;r.plan.original_id=e==fnfErr?file->id:c.hFileInfo.ioDirID;r.plan.original_modified=e==fnfErr?0:c.hFileInfo.ioFlMdDat;
  if(!e&&(c.hFileInfo.ioFlAttrib&17))return permErr;
  strcpy(r.plan.target,target);strcpy(r.plan.stage,target);char *leaf=strrchr(r.plan.stage,':')+1;
  snprintf(leaf,sizeof r.plan.stage-(leaf-r.plan.stage),".cr-%08lx-%08lx.new",(unsigned long)r.plan.original_id,(unsigned long)h->build);strcpy(r.plan.backup,r.plan.stage);strcpy(r.plan.backup+strlen(r.plan.backup)-3,"old");
  r.plan.build=h->build;r.plan.data_size=file->data;r.plan.resource_size=file->resource;r.plan.data_crc=file->data_crc;r.plan.resource_crc=file->resource_crc;r.plan.created=file->created;r.plan.modified=file->modified;memcpy(r.plan.metadata,file->finder,16);memcpy(r.plan.extra,file->extra,16);
  CInfoPBRec stage;e=update_info(r.plan.stage,&stage);
  if(e==fnfErr||!e)e=recovery_resume_copy(source,r.plan.stage,&r.file);
  if(e)return e;e=update_info(r.plan.stage,&stage);if(e)return e;r.plan.stage_id=stage.hFileInfo.ioDirID;
  e=partial?recovery_resume_plan(journal,&r.plan):recovery_plan_write(journal,&r.plan);if(e)return e;
 }
 /* A consumed stage is recognised by catalog identity and complete readback. */
 CInfoPBRec now,old;int a=update_info(r.plan.target,&now),b=update_info(r.plan.backup,&old);
 int installed=!a&&(uint32_t)now.hFileInfo.ioDirID==r.plan.stage_id&&!recovery_restore_check(&r,0);
 if(b!=fnfErr&&(b||(uint32_t)old.hFileInfo.ioDirID!=r.plan.original_id||old.hFileInfo.ioFlMdDat!=r.plan.original_modified))return b?b:paramErr;
 if(!installed){
  if(a==fnfErr&&!b){e=recovery_restore_closed(&r,2);if(!e)e=recovery_restore_move(&r,2,0);if(!e)e=recovery_restore_flush(&r);if(e)return e;}
  CInfoPBRec stage;e=update_info(r.plan.stage,&stage);if(e)return e;if((uint32_t)stage.hFileInfo.ioDirID!=r.plan.stage_id)return paramErr;
  UpdateOps ops={&r,recovery_restore_probe,recovery_restore_check,recovery_restore_closed,recovery_restore_move,recovery_restore_flush};UpdateResult result=update_commit(&ops);if(result.error)return result.error;
 }
 /* Full immutable snapshot is still available; this transaction's displaced
    failed version is no longer needed after successful restored-file readback. */
 e=update_info(r.plan.backup,&old);if(!e){if((uint32_t)old.hFileInfo.ioDirID!=r.plan.original_id)return paramErr;e=recovery_delete(r.plan.backup);}else if(e==fnfErr)e=0;
 if(!e&&locked)e=recovery_lock(target,1);
 if(!e)e=update_flush(target);if(!e)e=recovery_delete(journal);return e?e:update_flush(journal);
}
/* After all members and catalog are restored, retire only the failed normal
 * update's verified plan and its exact prior Core. A partial/unknown plan or
 * candidate is retained and reported rather than deleted by name alone. */
static int recovery_finish_restore(const RecoverySnapshot *s){
 int e=recovery_clean_tools(s);if(e)return e;
 char path[256],target[256];if(strlen(s->header.root)+strlen(UPDATE_PLAN_NAME)+13>sizeof path)return bdNamErr;
 strcpy(path,s->header.root);strcat(path,"etc:clarice:");strcat(path,UPDATE_PLAN_NAME);
 UpdatePlan plan;e=recovery_plan_read(path,&plan);
 if(!e){
  e=recovery_core_target(&s->header,target);if(e)return e;if(plan.kind||strcmp(target,plan.target)||plan.build<s->header.build)return paramErr;
  const RecoveryFile *core=0;for(uint32_t i=0;i<s->header.count;i++)if(!strcmp(s->files[i].name,"@core"))core=&s->files[i];if(!core)return paramErr;
  CInfoPBRec c;e=update_info(plan.backup,&c);if(!e){if((uint32_t)c.hFileInfo.ioDirID!=plan.original_id)return paramErr;e=recovery_check_file(plan.backup,core,0);if(!e)e=recovery_delete(plan.backup);}else if(e==fnfErr)e=0;if(e)return e;
  /* Candidate remains available for diagnosis; it is not the active file.
     A subsequent explicit update owns validation/removal of its own stage. */
  e=recovery_delete(path);if(e)return e;
 }else if(e!=fnfErr)return e;
 strcpy(path,s->header.root);strcat(path,"etc:clarice:update.incomplete");e=recovery_delete(path);if(e==fnfErr)e=0;return e?e:update_flush(path);
}
static int recovery_restore(const char *dir,char launch[256]){
 RecoverySnapshot s;int e=recovery_inventory(dir,&s,0);if(e)return e;e=recovery_verify(dir,&s);if(e){recovery_dispose(&s);return e;}
 char journal[256];e=recovery_join(dir,RECOVERY_RESTORE_PLAN,journal);UpdatePlan pending;
 if(!e){e=recovery_plan_read(journal,&pending);if(e==fnfErr||e==paramErr||e==eofErr)e=0;else if(!e){int found=0;for(uint32_t i=0;i<s.header.count;i++){char target[256];if(!strcmp(s.files[i].name,"@helper"))continue;e=recovery_target(&s.header,s.files[i].name,target);if(e)break;if(!strcmp(target,pending.target)){found=1;e=recovery_restore_one(dir,&s.header,&s.files[i]);break;}}if(!e&&!found)e=paramErr;}}
 /* Restore the catalog last, after every member it describes. */
 for(int pass=0;!e&&pass<2;pass++)for(uint32_t i=0;!e&&i<s.header.count;i++){
  const char *name=s.files[i].name;if(!strcmp(name,"@helper")||(!strcmp(name,"@catalog"))!=pass)continue;
  e=recovery_restore_one(dir,&s.header,&s.files[i]);
 }
 if(!e)e=recovery_finish_restore(&s);
 if(!e)e=recovery_core_target(&s.header,launch);recovery_dispose(&s);return e;
}
#endif
