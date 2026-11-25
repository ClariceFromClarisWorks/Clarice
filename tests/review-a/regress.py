"""Focused tests of production installer branches and repository retry checks."""
from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parents[2]
o=(r/'src/offline-install.inc').read_text();h=(r/'src/offline-recovery.inc').read_text();repo=(r/'src/commands/repository-recovery.inc').read_text()
def function(s,name):
 start=s.index('static int '+name+'(');i=s.index('{',start);depth=1;j=i+1
 while depth:
  depth+=(s[j]=='{')-(s[j]=='}');j+=1
 return s[start:j]
code=r'''
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define paramErr -50
#define fnfErr -43
#define bdNamErr -37
#define PATH_CAP 256
#include "src/update/transaction.h"
#include "src/update/repository-marker.h"
static const char root_path[]="Disk:Data:";
static const char *standard_commands[]={"ls","cat",0};
static int missing,marker,info_error,corrupt,staged=1;
static unsigned type='APPL',creator='CLUP';
typedef struct {struct {unsigned ioFlAttrib;struct {unsigned fdType,fdCreator;}ioFlFndrInfo;}hFileInfo;} CInfoPBRec;
static int info(const char *p,CInfoPBRec *c){memset(c,0,sizeof *c);c->hFileInfo.ioFlFndrInfo.fdType=type;c->hFileInfo.ioFlFndrInfo.fdCreator=creator;if(info_error)return info_error;if(strstr(p,"update.incomplete"))return marker?0:fnfErr;return missing?fnfErr:0;}
static int command_paths(const char *n,char *d,char*s,char*b){strcpy(d,n);return 0;}
static void errsay(const char *s){(void)s;}
static unsigned offline_helper_data=4,offline_helper_data_crc=123,offline_helper_resource=100,offline_helper_resource_crc=456;
static int update_hash_fork(const char*p,int f,uint32_t*c,unsigned n){*c=f?456:123;if(corrupt==f+1)(*c)++;return corrupt==3?-39:0;}
'''+function(h,'offline_helper_check')+r'''
static int cleanup(void){UpdateResult result={-128,0,0,0,1};
'''+o[o.index('if(result.error){'):o.index('\n say("Installation complete.')]+r'''
return 0;}
typedef struct {char name[32];unsigned core,size,crc;} Wanted;
typedef struct RepoItem {Wanted wanted;struct RepoItem *next;} RepoItem;
typedef struct {char name[32];unsigned id,data,data_crc;} RecoveryFile;
typedef struct {struct {char root[256];unsigned count;}header;RecoveryFile *files;} RecoverySnapshot;
static RecoveryFile files[2]={{"ls",1,10,20},{"cat",0,30,40}};
static int verified,disposed,verify_error;
static char marker_text[768];
#define REPO_MARKER "marker"
static int read_text(const char*p,char*t,unsigned n){snprintf(t,n,"%s",marker_text);return 0;}
static int recovery_inventory(const char*p,RecoverySnapshot*s,int w){strcpy(s->header.root,"root");s->header.count=2;s->files=files;return 0;}
static int recovery_verify(const char*p,RecoverySnapshot*s){verified++;return verify_error;}
static void recovery_dispose(RecoverySnapshot*s){disposed++;}
static int retry(RepoItem *items,unsigned index_size,unsigned index_crc){int e=0;char root[]="root";struct {char path[1][256];}rotation={0};
'''+repo[repo.index(' if(!e){\n  char text[768]'):repo.index('\n if(e!=fnfErr)return e;')]+r'''
return -1;}
int main(void){
 assert(cleanup()==-128&&staged);puts("PASS cancelled candidate retained for durable-plan retry");
 assert(!offline_helper_check("helper"));type='TEXT';assert(offline_helper_check("helper")==paramErr);type='APPL';creator='XXXX';assert(offline_helper_check("helper")==paramErr);creator='CLUP';
 for(corrupt=1;corrupt<=3;corrupt++)assert(offline_helper_check("helper")<0);corrupt=0;assert(!offline_helper_check("helper"));puts("PASS helper type/creator, both fork hashes and length failures");
 RepoItem cat={{"cat",0,30,40},0},ls={{"ls",0,10,20},&cat};
 assert(repo_marker_header(marker_text,sizeof marker_text,0,100,200)>0);assert(!retry(&ls,100,200));int before=verified;assert(retry(&ls,101,200)==paramErr&&verified==before);assert(retry(&ls,100,201)==paramErr&&verified==before);
 cat.wanted.crc++;assert(retry(&ls,100,200)==paramErr);cat.wanted.crc--;strcpy(cat.wanted.name,"new");assert(retry(&ls,100,200)==paramErr);strcpy(cat.wanted.name,"cat");
 strcpy(marker_text,"CLARICE-UPDATE-INCOMPLETE 1\n");assert(!retry(&ls,100,200));cat.wanted.size++;assert(retry(&ls,100,200)==paramErr);cat.wanted.size--;verify_error=-36;assert(retry(&ls,100,200)==-36);verify_error=0;
 assert(verified==disposed);puts("PASS bound retry rejects changed index; legacy retry validates snapshot membership/payload; cleanup");
 const char *body;unsigned size,crc;int bound;
 for(int ready=0;ready<2;ready++){int n=repo_marker_header(marker_text,sizeof marker_text,ready,100,200);if(ready)strcat(marker_text,"core row\nDisk:Clarice\n");assert(repo_marker_parse(marker_text,&size,&crc,&body,&bound)==ready+1&&bound&&size==100&&crc==200);for(int i=0;i<n;i++){char t[768];memcpy(t,marker_text,i);t[i]=0;assert(repo_marker_parse(t,&size,&crc,&body,&bound)<0);}}
 strcpy(marker_text,"CLARICE-UPDATE-READY 1\ncore row\nDisk:Clarice\n");assert(repo_marker_parse(marker_text,&size,&crc,&body,&bound)==2&&!bound&&!strcmp(body,"core row\nDisk:Clarice\n"));
 puts("PASS old/new markers, Ready payload and all header truncations");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'test.c';p.write_text(code)
 subprocess.run(['cc','-Wno-multichar','-fsanitize=address,undefined','-I',str(r),str(p),'-o',d+'/test'],check=True)
 subprocess.run([d+'/test'],check=True)
