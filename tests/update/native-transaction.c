/* Disposable native HFS fault test; never touches an installed application. */
#include <Quickdraw.h>
#include <Fonts.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <stdio.h>
#include "../../src/update/native-files.h"
#include "../../src/update/open-guard.h"
#include "../../src/update/transaction.h"
static char names[3][256];static int moves,fail_second;
static int probe(void *c,int n){(void)c;CInfoPBRec p;int e=update_info(names[n],&p);return e==fnfErr?0:e?e:1;}
static int verify(void *c,int n){(void)c;short ref;unsigned char b[4];int e=update_open(names[n],1,0,&ref);if(e)return e;long count=sizeof b;e=FSRead(ref,&count,b);int close=FSClose(ref);return e?e:close?close:count!=4||memcmp(b,"NEWr",4)?paramErr:0;}
static int closed(void *c,int n){(void)c;int busy,e=update_open_guard(names[n],&busy);return e?e:busy?fBsyErr:0;}
static int move(void *c,int a,int b){(void)c;if(++moves==2&&fail_second)return ioErr;return update_rename(names[a],names[b]);}
static int flush(void *c){(void)c;return update_flush(names[0]);}
static int create(const char *path,int newer){unsigned char p[256];update_pstr(p,path);int e=HCreate(0,0,p,'CLRC','TEXT');if(e)return e;
 for(int fork=0;fork<2;fork++){short ref;e=update_open(path,fork,1,&ref);if(e)return e;const char *s=newer?(fork?"NEWr":"NEWd"):(fork?"OLDr":"OLDd");long n=4;e=FSWrite(ref,&n,s);int x=FSClose(ref);if(e||x||n!=4)return e?e:x?x:ioErr;}return update_flush(path);}
static int exact(const char *path,int newer){for(int fork=0;fork<2;fork++){short ref;unsigned char b[4];int e=update_open(path,fork,0,&ref);if(e)return 0;long n=4;e=FSRead(ref,&n,b);int x=FSClose(ref);const char *s=newer?(fork?"NEWr":"NEWd"):(fork?"OLDr":"OLDd");if(e||x||n!=4||memcmp(b,s,4))return 0;}return 1;}
int main(void){InitGraf(&qd.thePort);InitFonts();InitWindows();InitMenus();TEInit();InitDialogs(0);InitCursor();FILE *log=fopen("Sys6Unix:HIL Results:native-txn.txt","w");if(!log)return 1;
 for(int fault=0;fault<2;fault++){snprintf(names[0],256,"Sys6Unix:Txn fixture:target%d",fault);snprintf(names[1],256,"Sys6Unix:Txn fixture:stage%d",fault);snprintf(names[2],256,"Sys6Unix:Txn fixture:backup%d",fault);int e=create(names[0],0);if(!e)e=create(names[1],1);if(e){fprintf(log,"setup error=%d\n",e);break;}moves=0;fail_second=fault;UpdateOps o={0,probe,verify,closed,move,flush};UpdateResult r=update_commit(&o);int original=exact(fault?names[0]:names[2],0),candidate=exact(fault?names[1]:names[0],1);fprintf(log,"fault=%d result=%d rollback=%d original_exact=%d candidate_exact=%d moves=%d\n",fault,r.error,r.rollback_error,original,candidate,moves);}
 fclose(log);FlushVol(0,0);ExitToShell();return 0;}
