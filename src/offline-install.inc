/* Offline launcher only. Reuses the CLX installer, cp's fork-preserving staging,
   and the native updater transaction. Never linked into ordinary Clarice. */
#include "update/transaction.h"
#include "update/open-guard.h"
#define UPDATE_VERIFY_POLL() (app_yield()?userCanceledErr:0)
#include "update/verify.h"
#define RECOVERY_POLL() (app_yield()?userCanceledErr:0)
#include "offline-recovery.inc"
#include "update/recovery-restore.h"
#include "offline-maintenance.inc"
static UpdatePlan offline_plan;
static const char *offline_slot(int n){return n==0?offline_plan.target:n==1?offline_plan.stage:offline_plan.backup;}
static int offline_probe(void *c,int slot){(void)c;CInfoPBRec p;int e=info(offline_slot(slot),&p);return e==fnfErr?0:e?e:1;}
static int offline_check(void *c,int slot){(void)c;return slot==1?update_verify_candidate(offline_slot(slot),&offline_plan):update_verify_structure(offline_slot(slot),&offline_plan);}
static int offline_closed(void *c,int slot){(void)c;int busy=1,e=update_open_guard(offline_slot(slot),&busy);return e?e:busy?fBsyErr:0;}
static int offline_move(void *c,int a,int b){(void)c;return cli_native_rename(offline_slot(a),offline_slot(b));}
static int offline_flush(void *c){(void)c;return command_flush(offline_plan.target);}
static int offline_marker(const char *text){
 char path[256];if(strlen(root_path)+30>sizeof path)return bdNamErr;strcpy(path,root_path);strcat(path,"etc:clarice:update.incomplete");
 CInfoPBRec c;int e=info(path,&c);if(e==fnfErr)e=cli_native_create(path,'TEXT','CLRC');if(e)return e;
 short ref;e=cli_native_open(path,0,1,&ref);if(e)return e;e=SetEOF(ref,0);if(!e)e=recovery_write(ref,text,strlen(text));int x=FSClose(ref);if(!e)e=x;return e?e:update_flush(path);
}
static int offline_ready(const char *target,uint32_t build){
 RepoRecord core;memset(&core,0,sizeof core);core.core=1;strcpy(core.name,"Clarice");core.version=build;char row[256],text[768];int e=repo_line(&core,row,sizeof row);if(e)return e;
 int n=repo_marker_header(text,sizeof text,1,RELEASE_MANIFEST_SIZE,offline_manifest_crc);if(n<0)return n;snprintf(text+n,sizeof text-n,"%s%s\n",row,target);return offline_marker(text);
}
/* The same durable Core plan used by the independent helper makes rerunning
 * the offline installer finish an interrupted handoff, not start a new one. */
static int offline_resume(const char *target,int *finished){
 *finished=0;char path[256];if(strlen(root_path)+strlen(UPDATE_PLAN_NAME)+13>sizeof path)return bdNamErr;strcpy(path,root_path);strcat(path,"etc:clarice:");strcat(path,UPDATE_PLAN_NAME);
 UpdatePlan pending;int e=recovery_plan_read(path,&pending);
 if(e==fnfErr){
  strcpy(path,root_path);strcat(path,"etc:clarice:update.incomplete");short ref;e=update_open(path,0,0,&ref);if(e==fnfErr)return 0;if(e)return e;
  char text[768];long bytes;e=GetEOF(ref,&bytes);if(!e&&(bytes<0||bytes>=(long)sizeof text))e=paramErr;if(!e)e=recovery_read(ref,text,bytes);int x=FSClose(ref);if(!e)e=x;if(e)return e;text[bytes]=0;
  const char *body;uint32_t size,crc;int bound;int kind=repo_marker_parse(text,&size,&crc,&body,&bound);if(kind<0)return kind;
  if(bound&&(size!=RELEASE_MANIFEST_SIZE||crc!=offline_manifest_crc)){errsay("Resume with the original kit or repository; this source differs.\n");return paramErr;}
  if(kind!=2)return 0;
  char *row=(char*)body,*end=strchr(row,'\n');if(!end)return paramErr;*end++=0;RepoRecord core;e=repo_record(row,&core,1);if(e||!core.core)return paramErr;char *nl=strchr(end,'\n');if(!nl||nl[1])return paramErr;*nl=0;if(strcmp(end,target))return paramErr;
  uint32_t build;unsigned system;e=update_stamp(target,&build,&system);if(e==fnfErr)return 0;if(e)return e;if(build==core.version)*finished=1;return 0;
 }if(e)return e;
 if(pending.kind||strcmp(pending.target,target))return paramErr;offline_plan=pending;
 char recovery[256];strcpy(recovery,root_path);strcat(recovery,"etc:clarice:");strcat(recovery,RECOVERY_CURRENT);RecoverySnapshot snapshot;e=recovery_inventory(recovery,&snapshot,0);if(e)return e;e=strcmp(snapshot.header.root,root_path)?paramErr:recovery_verify(recovery,&snapshot);recovery_dispose(&snapshot);if(e)return e;
 CInfoPBRec current,previous;int a=info(target,&current),b=info(pending.backup,&previous);
 int saved=!b&&(uint32_t)previous.hFileInfo.ioDirID==pending.original_id&&previous.hFileInfo.ioFlMdDat==pending.original_modified;
 if(!a&&!update_verify_candidate(target,&pending)){e=offline_ready(target,pending.build);if(!e)*finished=1;return e;}
 if(a==fnfErr&&saved){e=offline_closed(0,2);if(!e)e=offline_move(0,2,0);if(!e)e=offline_flush(0);if(e)return e;}
 e=info(target,&current);if(e)return e;if((uint32_t)current.hFileInfo.ioDirID!=pending.original_id||current.hFileInfo.ioFlMdDat!=pending.original_modified)return paramErr;
 e=offline_ready(target,pending.build);if(e)return e;UpdateOps ops={0,offline_probe,offline_check,offline_closed,offline_move,offline_flush};UpdateResult result=update_commit(&ops);if(!result.error)*finished=1;return result.error;
}
int shell_install_offline(char target[256],int repair){
 unsigned steps=sizeof standard_commands/sizeof *standard_commands-1+4;
 app_install_progress("Checking installation and recovery",1,steps);
 char own[256],source[PATH_CAP],kit[PATH_CAP];CInfoPBRec original,cat;
 int e=update_application_path(own);if(e)return e;char *leaf=strrchr(own,':');if(!leaf)return bdNamErr;*++leaf=0;
 if(strlen(own)+10>=sizeof source)return bdNamErr;
 strcpy(source,own);strcat(source,"Clarice");strcpy(kit,own);strcat(kit,"Utilities");
 memset(&offline_plan,0,sizeof offline_plan);e=locations_core(root_path,offline_plan.target);if(e)return e;strcpy(target,offline_plan.target);

 strcpy(offline_plan.stage,target);strcpy(strrchr(offline_plan.stage,':')+1,"Clarice Installing");
 strcpy(offline_plan.backup,target);strcpy(strrchr(offline_plan.backup,':')+1,"Clarice Previous");
 char origin[256];strcpy(origin,target);int migrating=0;
 CInfoPBRec destination;e=info(target,&destination);
 if(e==fnfErr){size_t n=strlen(root_path);if(n<13||strcmp(root_path+n-13,"Clarice Data:"))return paramErr;
  memcpy(origin,root_path,n-13);strcpy(origin+n-13,"Clarice");e=info(origin,&destination);if(!e)migrating=1;else if(e==fnfErr){e=0;strcpy(origin,target);}if(e)return e;
 }else if(e)return e;
 unsigned char manifest[RELEASE_MANIFEST_SIZE];char media[256];strcpy(media,own);media[strlen(media)-1]=0;
 e=offline_manifest(media,manifest);if(e)return e;
 uint32_t build=0,installed=0;unsigned system=0,oldsystem=0;
 int source_busy=1;e=update_open_guard(source,&source_busy);if(e||source_busy)return e?e:fBsyErr;
 e=update_stamp(source,&build,&system);if(e)return e;if(build!=update_be32(manifest+8))return paramErr;SysEnvRec env;e=SysEnvirons(1,&env);if(e)return e;
 if(system>(unsigned short)env.systemVersion)return paramErr;
 e=info(source,&cat);if(e)return e;if(cat.hFileInfo.ioFlFndrInfo.fdType!='APPL'||cat.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC')return paramErr;
 int present=migrating?1:offline_probe(0,0),copy=1;if(present<0)return present;
 if(present){e=info(origin,&original);if(e)return e;
  if(original.hFileInfo.ioFlAttrib&17||original.hFileInfo.ioFlFndrInfo.fdType!='APPL'||original.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC')return paramErr;
  {int busy=1;e=update_open_guard(origin,&busy);if(e||busy)return e?e:fBsyErr;}
  e=update_stamp(origin,&installed,&oldsystem);if(e)return e;copy=build>installed;
 }
 if(present&&build<installed){errsay("This kit is older than the installed Core. No managed files changed.\n");return paramErr;}
 if(repair&&(!present||build!=installed||migrating)){errsay("Repair requires the kit matching the installed Core. Repair that version before upgrading.\n");return paramErr;}
 e=offline_pending();if(e){errsay("Use the original kit to resume this installation.\n");return e;}
 OfflineTool tools[RELEASE_COUNT];memset(tools,0,sizeof tools);uint32_t catalog_core;
 e=offline_catalog_read(tools,&catalog_core);if(e)return e;
 int changes;e=offline_tools_plan(kit,manifest,tools,repair,&changes);if(e)return e;
 {int finished=0;e=offline_resume(target,&finished);if(e)return e;if(finished){if(changes)return paramErr;return 0;}}
 if(present&&catalog_core&&catalog_core!=installed){errsay("Installed Core does not match its catalog. Use existing recovery first.\n");return paramErr;}
 if(present&&!copy&&!migrating&&!changes){
  /* Adopt exact legacy files into the catalog; do not rotate recovery. */
  e=offline_tools_install(kit,manifest,tools,repair);return e?e:offline_ready(target,installed);
 }
 if(present){
  char media[256];strcpy(media,own);size_t n=strlen(media);if(n&&media[n-1]==':')media[n-1]=0;
  if(!repair){e=offline_recovery(origin,media,installed);if(e)return e;}
  if(migrating){RecoveryFile previous;e=recovery_copy(origin,target,"@core",&previous);if(e)return e;e=info(target,&original);if(e)return e;}
  offline_plan.original_id=original.hFileInfo.ioDirID;offline_plan.original_modified=original.hFileInfo.ioFlMdDat;
  char *name=strrchr(offline_plan.stage,':')+1;snprintf(name,sizeof offline_plan.stage-(name-offline_plan.stage),".cu-%08lx-%08lx.new",(unsigned long)offline_plan.original_id,(unsigned long)build);
  strcpy(offline_plan.backup,offline_plan.stage);strcpy(offline_plan.backup+strlen(offline_plan.backup)-3,"old");
 }
 if(copy&&(e=command_absent(offline_plan.backup))){errsay("Installer: staged files retained; use Recover Clarice or inspect before retry.\n");return e;}
 {char marker[128];int n=repo_marker_header(marker,sizeof marker,0,RELEASE_MANIFEST_SIZE,offline_manifest_crc);if(n<0)return n;e=offline_marker(marker);if(e)return e;}
 e=offline_tools_install(kit,manifest,tools,repair);if(e)return e;
 if(!copy){say("Installed core is the same build or newer; kept unchanged.\n");return offline_ready(target,installed);}
 app_install_progress("Copying and checking Clarice Core",steps-2,steps);
 say("Copying and checking Clarice Core (both forks)...\n");
 offline_plan.build=build;offline_plan.data_size=cat.hFileInfo.ioFlLgLen;offline_plan.resource_size=cat.hFileInfo.ioFlRLgLen;
 e=update_hash_fork(source,0,&offline_plan.data_crc,offline_plan.data_size);if(e)return e;
 e=update_hash_fork(source,1,&offline_plan.resource_crc,offline_plan.resource_size);if(e)return e;
 RecoveryFile image;e=recovery_stat(source,"@core",&image);if(e)return e;image.data_crc=offline_plan.data_crc;image.resource_crc=offline_plan.resource_crc;
 e=recovery_resume_copy(source,offline_plan.stage,&image);if(e)return e;
 e=info(offline_plan.stage,&cat);if(e)return e;
 if(present){cat.hFileInfo.ioFlFndrInfo=original.hFileInfo.ioFlFndrInfo;cat.hFileInfo.ioFlXFndrInfo=original.hFileInfo.ioFlXFndrInfo;cat.hFileInfo.ioFlCrDat=original.hFileInfo.ioFlCrDat;e=cli_native_set(offline_plan.stage,&cat);if(e)return e;}
 offline_plan.created=cat.hFileInfo.ioFlCrDat;offline_plan.modified=cat.hFileInfo.ioFlMdDat;
 memcpy(offline_plan.metadata,&cat.hFileInfo.ioFlFndrInfo,16);memcpy(offline_plan.extra,&cat.hFileInfo.ioFlXFndrInfo,16);
 if(present){char planpath[256];if(strlen(root_path)+strlen(UPDATE_PLAN_NAME)+13>256)return bdNamErr;strcpy(planpath,root_path);strcat(planpath,"etc:clarice:");strcat(planpath,UPDATE_PLAN_NAME);e=recovery_plan_write(planpath,&offline_plan);if(e)return e;}
 e=offline_ready(target,build);if(e)return e;
 app_install_progress("Activating verified installation",steps-1,steps);
 UpdateOps ops={0,offline_probe,offline_check,offline_closed,offline_move,offline_flush};
 UpdateResult result=update_commit(&ops);if(result.error){
  /* The durable plan may already refer to this stage. A cancelled check is
     retryable; never delete the candidate while leaving its plan behind. */
  errsay("Installer: Core staging and recovery files retained for retry.\n");
  return result.error;
 }
 say("Installation complete. Launching installed Clarice...\n");return 0;
}
