#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "../../src/update/collection.h"
static int files[4][3],has_old[4],journal,step,fail,crash,mode,unrelated,preferences;
static jmp_buf stop;
static int fault(void){++step;if(step==crash)longjmp(stop,1);return step==fail?-36:0;}
static int old(void *c,unsigned i){(void)c;return has_old[i];}
static int identify(void *c,unsigned i,int s){(void)c;int e=fault();return e?e:files[i][s];}
static int move(void *c,unsigned i,int a,int b){(void)c;int e=fault();if(e)return e;assert(files[i][a]&&!files[i][b]);files[i][b]=files[i][a];files[i][a]=0;return 0;}
static int flush(void *c){(void)c;return fault();}
static int logstate(void *c,int n){(void)c;int e=fault();if(e)return e;journal=n;return 0;}
static int cancel(void *c){(void)c;int e=fault();return mode&&e?-128:e;}
static CollectionOps ops={0,4,old,identify,move,flush,logstate,cancel};
static void reset(void){memset(files,0,sizeof files);for(int i=0;i<4;i++){has_old[i]=i!=1;files[i][0]=has_old[i];files[i][1]=2;}journal=step=fail=crash=mode=0;unrelated=123;preferences=456;}
static void intact(void){for(int i=0;i<4;i++){assert(files[i][0]==has_old[i]);assert(files[i][1]==2);assert(!files[i][2]);}assert(unrelated==123&&preferences==456);}
int main(void){
 reset();CollectionResult r=collection_commit(&ops);assert(!r.error&&r.committed&&journal==2);int n=step;
 for(int i=0;i<4;i++){assert(files[i][0]==2&&files[i][2]==has_old[i]);}
 for(int k=1;k<=n;k++){
  reset();fail=k;r=collection_commit(&ops);assert(r.error&&!r.committed);fail=0;
  if(journal==1)assert(!collection_recover(&ops));
  if(journal!=2)intact();else for(int i=0;i<4;i++)assert(files[i][0]==2);
 }
 for(int k=1;k<=n;k++){
  reset();crash=k;if(!setjmp(stop))collection_commit(&ops);crash=0;
  if(journal==1)assert(!collection_recover(&ops));
  if(journal!=2)intact();else for(int i=0;i<4;i++)assert(files[i][0]==2);
 }
 reset();files[2][1]=0;r=collection_commit(&ops);assert(r.error&&!journal&&files[0][0]==1);
 reset();files[2][0]=3;r=collection_commit(&ops);assert(r.error&&!journal&&files[2][0]==3);
 reset();files[1][2]=1;r=collection_commit(&ops);assert(r.error&&!journal);
 printf("PASS: successful mixed fresh/upgrade collection; %d injected errors and %d interruptions; missing/unknown/recovery conflicts; no unrelated changes\n",n,n);
}
