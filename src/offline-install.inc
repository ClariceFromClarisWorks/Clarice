/* Offline launcher only. Reuses the CLX installer, cp's fork-preserving staging,
   and the native updater transaction. Never linked into ordinary Clarice. */
#include "update/transaction.h"
#include "update/open-guard.h"
#define UPDATE_VERIFY_POLL() (app_yield()?userCanceledErr:0)
#include "update/verify.h"
static UpdatePlan offline_plan;
static const char *offline_slot(int n){return n==0?offline_plan.target:n==1?offline_plan.stage:offline_plan.backup;}
static int offline_probe(void *c,int slot){(void)c;CInfoPBRec p;int e=info(offline_slot(slot),&p);return e==fnfErr?0:e?e:1;}
static int offline_check(void *c,int slot){(void)c;return slot==1?update_verify_candidate(offline_slot(slot),&offline_plan):update_verify_structure(offline_slot(slot),&offline_plan);}
static int offline_closed(void *c,int slot){(void)c;int busy=1,e=update_open_guard(offline_slot(slot),&busy);return e?e:busy?fBsyErr:0;}
static int offline_move(void *c,int a,int b){(void)c;return cli_native_rename(offline_slot(a),offline_slot(b));}
static int offline_flush(void *c){(void)c;return command_flush(offline_plan.target);}
int shell_install_offline(char target[256]){
 char own[256],source[PATH_CAP],kit[PATH_CAP],cp[PATH_CAP+8],from[PATH_CAP+8],to[PATH_CAP+8];CInfoPBRec original,cat;
 int e=update_application_path(own);if(e)return e;char *leaf=strrchr(own,':');if(!leaf)return bdNamErr;*++leaf=0;
 if(strlen(own)+10>=sizeof source)return bdNamErr;
 strcpy(source,own);strcat(source,"Clarice");strcpy(kit,own);strcat(kit,"Utilities");
 size_t n=strlen(root_path);if(n<13||strcmp(root_path+n-13,"Clarice Data:"))return paramErr;
 if(n+20>=sizeof offline_plan.target)return bdNamErr;
 memset(&offline_plan,0,sizeof offline_plan);memcpy(offline_plan.target,root_path,n-13);strcat(offline_plan.target,"Clarice");
 memcpy(offline_plan.stage,root_path,n-13);strcat(offline_plan.stage,"Clarice Installing");
 memcpy(offline_plan.backup,root_path,n-13);strcat(offline_plan.backup,"Clarice Previous");strcpy(target,offline_plan.target);
 uint32_t build=0,installed=0;unsigned system=0,oldsystem=0;
 int source_busy=1;e=update_open_guard(source,&source_busy);if(e||source_busy)return e?e:fBsyErr;
 e=update_stamp(source,&build,&system);if(e)return e;SysEnvRec env;e=SysEnvirons(1,&env);if(e)return e;
 if(system>(unsigned short)env.systemVersion)return paramErr;
 e=info(source,&cat);if(e)return e;if(cat.hFileInfo.ioFlFndrInfo.fdType!='APPL'||cat.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC')return paramErr;
 int present=offline_probe(0,0),copy=1;if(present<0)return present;
 if(present){e=info(target,&original);if(e)return e;
  if(original.hFileInfo.ioFlAttrib&17||original.hFileInfo.ioFlFndrInfo.fdType!='APPL'||original.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC')return paramErr;
  if((e=offline_closed(0,0)))return e;
  e=update_stamp(target,&installed,&oldsystem);if(e)return e;copy=build>installed;
 }
 if(copy&&((e=command_absent(offline_plan.stage))||(e=command_absent(offline_plan.backup)))){
  errsay("Installer: retained Clarice Installing/Previous needs inspection.\n");return e;
 }
 snprintf(from,sizeof from,"native:%s",kit);
 say("Installing standard utilities. Existing files are kept.\nControl-C cancels; run Install Clarice again to resume.\n");
 e=command_setup(from);if(e)return e;
 if(!copy){say("Installed core is the same build or newer; kept unchanged.\n");return 0;}
 say("Copying and checking Clarice Core (both forks)...\n");
 offline_plan.build=build;offline_plan.data_size=cat.hFileInfo.ioFlLgLen;offline_plan.resource_size=cat.hFileInfo.ioFlRLgLen;
 e=update_hash_fork(source,0,&offline_plan.data_crc,offline_plan.data_size);if(e)return e;
 e=update_hash_fork(source,1,&offline_plan.resource_crc,offline_plan.resource_size);if(e)return e;
 e=command_source(from,"cp",cp);if(e)return e;
 snprintf(from,sizeof from,"native:%s",source);snprintf(to,sizeof to,"native:%s",offline_plan.stage);
 char *args[]={"cp","-p",from,to,0};e=module_run(cp,4,args);if(e)return e;
 e=info(offline_plan.stage,&cat);if(e)return e;
 if(present){cat.hFileInfo.ioFlFndrInfo=original.hFileInfo.ioFlFndrInfo;cat.hFileInfo.ioFlXFndrInfo=original.hFileInfo.ioFlXFndrInfo;cat.hFileInfo.ioFlCrDat=original.hFileInfo.ioFlCrDat;e=cli_native_set(offline_plan.stage,&cat);if(e)return e;}
 offline_plan.created=cat.hFileInfo.ioFlCrDat;offline_plan.modified=cat.hFileInfo.ioFlMdDat;
 memcpy(offline_plan.metadata,&cat.hFileInfo.ioFlFndrInfo,16);memcpy(offline_plan.extra,&cat.hFileInfo.ioFlXFndrInfo,16);
 UpdateOps ops={0,offline_probe,offline_check,offline_closed,offline_move,offline_flush};
 UpdateResult result=update_commit(&ops);if(result.error){
  if(!result.installed&&!result.previous_retained&&!result.rollback_error){
   int cleanup=cli_native_delete(offline_plan.stage);if(cleanup)errsay("Installer: could not remove owned staging file.\n");
  }else errsay("Installer: core files retained for recovery in the System Folder.\n");
  return result.error;
 }
 say("Installation complete. Launching installed Clarice...\n");return 0;
}
