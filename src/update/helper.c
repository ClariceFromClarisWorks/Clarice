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
 trace(own);strcpy(path,own);char *leaf=strrchr(path,':');if(!leaf||(size_t)(leaf-path)+1+strlen(UPDATE_PLAN_NAME)>255){explain("Invalid updater location.");return 1;}strcpy(leaf+1,UPDATE_PLAN_NAME);
 unsigned char encoded[UPDATE_PLAN_SIZE];short ref=-1;e=update_open(path,0,0,&ref);
 if(!e){long size;e=GetEOF(ref,&size);if(!e&&size!=UPDATE_PLAN_SIZE)e=paramErr;if(!e){long n=sizeof encoded;e=FSRead(ref,&n,encoded);if(!e&&n!=sizeof encoded)e=ioErr;}int x=FSClose(ref);if(!e)e=x;}
 if(!e)e=update_plan_decode(encoded,&plan);trace(e?"plan failed":"plan read");
 if(e){explain("No valid confirmed update plan. Run clarice update from Clarice. Nothing was replaced.");return 1;}
 /* The only supported operation is replacement of the exact reviewed target.
    A missing target is an interrupted transaction, never a fresh installation. */
 if(probe(0,0)!=1){explain("Original application changed or is missing. Update stopped; staged and previous files were retained for recovery.");return 1;}
 trace("commit begin");UpdateOps ops={0,probe,verify,closed,move,flush};UpdateResult r=update_commit(&ops);
 trace(r.error?"commit failed":"commit complete");if(r.error){char text[240];snprintf(text,sizeof text,"Update stopped (Mac OS %d, rollback %d). No recovery file was deleted. See the retained update plan for recovery paths; do not overwrite them.",r.error,r.rollback_error);explain(text);return 1;}
 /* Remove only the validated plan we consumed. Previous application is retained.
    Helper remains available for subsequent updates, outside the application. */
 unsigned char pn[256];update_pstr(pn,path);e=HDelete(0,0,pn);if(!e)e=update_flush(path);
 if(e){explain("Application installed and previous version retained, but plan cleanup failed. Restart was not attempted.");return 1;}
 trace("relaunch");e=update_native_launch(plan.target);char text[240];snprintf(text,sizeof text,"Application installed; restart failed (Mac OS %d). Previous version remains recoverable. Open Clarice from Finder.",e);explain(text);return 1;
}
