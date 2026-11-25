/* Independent native updater. No network, shell, or resident service. */
#include <Quickdraw.h>
#include <Fonts.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <stdio.h>
#include "identity.h"
#include "native-launch.h"
#include "open-guard.h"
#include "transaction.h"
#include "verify.h"
#include "recovery-restore.h"
#ifdef UPDATE_HELPER_DIAGNOSTICS
static void trace(const char *s){FILE *f=fopen("Sys6Unix:HIL Results:helper-trace.txt","a");if(f){fprintf(f,"%lu %s\n",(unsigned long)TickCount(),s);fclose(f);}FlushVol(0,0);}
#else
#define trace(s) ((void)0)
#endif
static UpdatePlan plan;
static const char *slot(int n){return n==0?plan.target:n==1?plan.stage:plan.backup;}
static int probe(void *c,int n){(void)c;CInfoPBRec p;int e=update_info(slot(n),&p);if(e==fnfErr)return 0;if(e)return e;
 if(p.hFileInfo.ioFlAttrib&17)return permErr;
 if(n==0&&((uint32_t)p.hFileInfo.ioDirID!=plan.original_id||p.hFileInfo.ioFlMdDat!=plan.original_modified))return permErr;
 return 1;
}
/* Read both forks once in this new process before commit. With no yield or
 * content writes between renames, the post-commit check binds the same catalog
 * object and its structure instead of rereading its entire contents again. */
static uint32_t verified_id;
static int verify(void *c,int n){(void)c;CInfoPBRec cat;int e;
 if(n==1){e=update_verify_candidate(slot(n),&plan);if(e)return e;e=update_info(slot(n),&cat);if(!e)verified_id=cat.hFileInfo.ioDirID;return e;}
 e=update_info(slot(n),&cat);if(e)return e;if((uint32_t)cat.hFileInfo.ioDirID!=verified_id)return paramErr;
 return update_verify_structure(slot(n),&plan);
}
static int closed(void *c,int n){(void)c;int busy=1;int e=update_open_guard(slot(n),&busy);return e?e:busy?fBsyErr:0;}
static int move(void *c,int a,int b){(void)c;return update_rename(slot(a),slot(b));}
static int flush(void *c){(void)c;return update_flush(plan.target);}
static void explain(const char *s){unsigned char p[256];size_t n=strlen(s);if(n>255)n=255;p[0]=n;memcpy(p+1,s,n);trace(s);ParamText(p,0,0,0);StopAlert(151,0);}
int main(void){
 InitGraf(&qd.thePort);InitFonts();InitWindows();InitMenus();TEInit();InitDialogs(0);InitCursor();
 trace("started");char own[256],path[256];int e=update_application_path(own);if(e){explain("Cannot locate updater. No application was changed.");return 1;}
 trace(own);
 if(!strcmp(strrchr(own,':')+1,RECOVERY_HELPER)){
  strcpy(path,own);*strrchr(path,':')=0;
  ParamText((const unsigned char*)"\pRestore the managed previous Core, utilities and catalog? Quit Clarice first. Preferences and user files are not restored or removed.",0,0,0);
  if(CautionAlert(152,0)!=1)return 0;
  char launch[256];e=recovery_restore(path,launch);
  if(e){char text[240];snprintf(text,sizeof text,"Recovery stopped (Mac OS %d). The recovery snapshot is retained. Quit any running Clarice and run Recover Clarice again.",e);explain(text);return 1;}
  e=update_native_launch(launch);if(e)explain("Files restored; open Clarice at its installed location.");return e?1:0;
 }
 strcpy(path,own);char *leaf=strrchr(path,':');if(!leaf||(size_t)(leaf-path)+1+strlen(UPDATE_PLAN_NAME)>255){explain("Invalid updater location.");return 1;}strcpy(leaf+1,UPDATE_PLAN_NAME);
 unsigned char encoded[UPDATE_PLAN_SIZE];short ref=-1;e=update_open(path,0,0,&ref);
 if(!e){long size;e=GetEOF(ref,&size);if(!e&&size!=UPDATE_PLAN_SIZE)e=paramErr;if(!e){long n=sizeof encoded;e=FSRead(ref,&n,encoded);if(!e&&n!=sizeof encoded)e=ioErr;}int x=FSClose(ref);if(!e)e=x;}
 if(!e)e=update_plan_decode(encoded,&plan);if(!e&&plan.kind)e=paramErr;trace(e?"plan failed":"plan read");
 if(e){explain("No valid confirmed update plan. Run clarice update from Clarice. Nothing was replaced.");return 1;}
 /* Resume only the exact confirmed native-file replacement. Completed tool
    installations are unrelated and are never rolled back here. */
 CInfoPBRec target,backup;int te=update_info(plan.target,&target),be=update_info(plan.backup,&backup);
 int previous=!be&&(uint32_t)backup.hFileInfo.ioDirID==plan.original_id&&backup.hFileInfo.ioFlMdDat==plan.original_modified;
 int committed=0;
 if(!te&&previous&&!update_verify_candidate(plan.target,&plan))committed=1;
 if(te==fnfErr&&previous){e=closed(0,2);if(!e)e=update_rename(plan.backup,plan.target);if(!e)e=update_flush(plan.target);if(e){explain("Could not restore the previous application; recovery files retained.");return 1;}}
 if(!committed){
  if(probe(0,0)!=1){explain("Original application changed or is missing. Recovery files retained.");return 1;}
  trace("commit begin");UpdateOps ops={0,probe,verify,closed,move,flush};UpdateResult r=update_commit(&ops);
  if(r.error){char text[240];snprintf(text,sizeof text,"Update stopped (Mac OS %d, recovery %d). Previous and staged files retained. Reopen Clarice and run clarice update -y to resume.",r.error,r.rollback_error);explain(text);return 1;}
 }
 /* Remove only after the installed Core confirms its catalog/snapshot.
    Keep the consumed plan through relaunch so cleanup knows exact ownership. */
 trace("relaunch");e=update_native_launch(plan.target);char text[240];snprintf(text,sizeof text,"Application installed; restart failed (Mac OS %d). Previous version remains recoverable. Open Clarice from Finder.",e);explain(text);return 1;
}
