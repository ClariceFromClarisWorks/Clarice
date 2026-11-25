"""Production remove + namespace conversion, with disposable in-memory HFS entries."""
from pathlib import Path
import subprocess,tempfile
root=Path.cwd();ns=(root/'src/namespace.inc').read_text()
def function(s,name):
 start=s.rfind('\n',0,s.index(name+'('))+1;a=s.index('{',start);n=1;i=a+1
 while n:
  n+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[start:i]+'\n'
pre=r'''
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#define PATH_CAP 512
enum {bdNamErr=-37,fnfErr=-43,dirNFErr=-120,paramErr=-50,memFullErr=-108,nsvErr=-35};
typedef struct {struct {int ioFlAttrib;short ioVRefNum;} hFileInfo;struct {long ioDrDirID;} dirInfo;int identity;} CInfoPBRec;
static char root_path[]="Disk:parent:",unix_cwd[]="/";
static const char *env_get(const char *s){(void)s;return "/home";}
static int volume_unique(const char*s){return strcmp(s,"Disk")?nsvErr:0;}
struct Node {const char*path,*target;int directory,alive;unsigned long metadata;char data[8],resource[8];};
static const struct Node seed[]={
 {"Disk:parent:",0,1,1,1,"ROOT","RF"},
 {"Disk:parent:keep",0,1,1,2,"DIR","RF"},
 {"Disk:parent:keep:sentinel",0,0,1,0x12345678,"DATA","FORK"},
 {"Disk:parent:keep/",0,0,1,4,"FILE","RF"},
 {"Disk:parent:mid/name",0,0,1,5,"FILE","RF"},
 {"Disk:parent:/lead",0,0,1,6,"FILE","RF"},
 {"Disk:parent:dirlink","/keep",0,1,7,"LINK","RF"},
 {"Disk:parent:dangling","/absent",0,1,8,"LINK","RF"},
 {"Disk:parent:folder/",0,1,1,9,"DIR","RF"},
 {"Disk:parent:folder/:child",0,0,1,10,"DATA","RF"}};
static struct Node nodes[10];static int calls,fail_at,live,allocs,alloc_fail,polls,cancel_at,recursive=1,force;
static int fault(void){return ++calls==fail_at?-36:0;}
static int info(const char*s,CInfoPBRec*p){int e=fault();if(e)return e;for(int i=0;i<10;i++)if(nodes[i].alive&&!strcasecmp(s,nodes[i].path)){memset(p,0,sizeof*p);p->identity=i;p->hFileInfo.ioFlAttrib=nodes[i].directory?16:0;p->hFileInfo.ioVRefNum=-1;p->dirInfo.ioDrDirID=i;return 0;}return fnfErr;}
static int isdir(const CInfoPBRec*p){return !!(p->hFileInfo.ioFlAttrib&16);}
static int is_link(const CInfoPBRec*p){return nodes[p->identity].target!=0;}
static int link_target(const char*s,char*p){CInfoPBRec b;int e=info(s,&b);if(!e)strcpy(p,nodes[b.identity].target);return e;}
static int app_yield(void){return ++polls==cancel_at;}
static void file_progress_current(const char*s){(void)s;}
static void file_progress_show(int n){(void)n;}
static void file_progress_complete(void){}
static void *cli_alloc(size_t n){if(++allocs==alloc_fail)return NULL;void*p=malloc(n);if(p)live++;return p;}
static void cli_free(void*p){if(p){live--;free(p);}}
static int cli_native_delete(const char*s){CInfoPBRec b;int e=info(s,&b);if(e)return e;if(isdir(&b)){size_t n=strlen(s);for(int i=0;i<10;i++)if(nodes[i].alive&&!strncmp(s,nodes[i].path,n)&&nodes[i].path[n]==':')return -47;}nodes[b.identity].alive=0;return 0;}
static int directory_nth(short vol,long id,int index,char*leaf,CInfoPBRec*p){assert(vol==-1&&index==1);int e=fault();if(e)return e;size_t n=strlen(nodes[id].path);for(int i=0;i<10;i++)if(nodes[i].alive&&!strncmp(nodes[id].path,nodes[i].path,n)&&nodes[i].path[n]==':'&&!strchr(nodes[i].path+n+1,':')){strcpy(leaf,nodes[i].path+n+1);return info(nodes[i].path,p);}return fnfErr;}
'''
helpers=''.join(function(ns,n) for n in ('same_name','encode_name','decode_name','native_path','unix_from_native','parent_path','resolve_full','resolve_ex','protected_path'))
h=Path('src/commands/mac-files.h').read_text();helpers+=''.join(function(h,n) for n in ('checked','join_path'))
test=r'''
static void reset(void){assert(!live);memcpy(nodes,seed,sizeof nodes);calls=fail_at=allocs=alloc_fail=polls=cancel_at=force=0;}
static void intact(int i){assert(!memcmp(&nodes[i],&seed[i],sizeof nodes[i]));}
static void except(int first,int second){for(int i=0;i<10;i++)if(i!=first&&i!=second)intact(i);assert(!live);}
int main(int argc,char**argv){(void)argv;reset();int rc=remove_one("native:Disk:parent:keep/",0,0);
 if(argc>1){assert(!rc&&!nodes[1].alive&&!nodes[2].alive&&nodes[3].alive);puts("REPRODUCED: native keep/ deletes sibling keep and sentinel on1968");return 0;}
 assert(!rc&&!nodes[3].alive);except(3,-1);
 const char *names[]={"native:Disk:parent:mid/name","native:Disk:parent:/lead","/keep%2F"};int ids[]={4,5,3};
 for(int i=0;i<3;i++){reset();assert(!remove_one(names[i],0,0)&&!nodes[ids[i]].alive);except(ids[i],-1);}
 reset();assert(!remove_one("native:Disk:parent:folder/",0,0)&&!nodes[8].alive&&!nodes[9].alive);except(8,9);
 const char *dirs[]={"/keep///","native:Disk:parent:keep:"};for(int i=0;i<2;i++){reset();assert(!remove_one(dirs[i],0,0)&&!nodes[1].alive&&!nodes[2].alive);except(1,2);}
 const char *bad[]={"/dirlink/","/dirlink///","/dirlink/.///","native:Disk:parent:dirlink:","/dangling/","/keep/.///","","native:","/absent","native:Disk:parent:absent/"};
 for(unsigned i=0;i<sizeof bad/sizeof*bad;i++){reset();assert(remove_one(bad[i],0,0));except(-1,-1);}
 for(int i=6;i<=7;i++){reset();assert(!remove_one(i==6?"/dirlink":"/dangling",0,0)&&!nodes[i].alive);except(i,-1);}
 reset();force=1;assert(remove_one("/dirlink/",0,0));except(-1,-1);
 reset();assert(!remove_one("native:Disk:parent:keep/",0,0));int bounds=calls;
 for(int i=1;i<=bounds;i++){reset();fail_at=i;assert(remove_one("native:Disk:parent:keep/",0,0));except(-1,-1);fail_at=0;assert(!remove_one("native:Disk:parent:keep/",0,0));except(3,-1);}
 reset();alloc_fail=1;assert(remove_one("/keep/",0,0)==memFullErr);except(-1,-1);alloc_fail=0;assert(!remove_one("/keep/",0,0));except(1,2);
 for(int i=1;i<=3;i++){reset();cancel_at=i;rc=remove_one("native:Disk:parent:folder/",0,0);assert(rc==130&&!live);for(int j=0;j<8;j++)intact(j);cancel_at=0;force=1;assert(!remove_one("native:Disk:parent:folder/",0,0));except(8,9);}
 puts("PASS native literal slashes, exact sibling protection, Unix/native directory/link suffixes, missing operands, native faults, allocation failure, cancellation, retry and cleanup");
}
'''
for label,s in [('baseline',subprocess.check_output(['git','show','0ab807e:src/commands/files.c'],text=True)),('fixed',Path('src/commands/files.c').read_text())]:
 body=s[s.index('typedef struct RemoveFrame'):s.index('static int temporary_sibling')]
 with tempfile.TemporaryDirectory() as d:
  p=Path(d);(p/'test.c').write_text(pre+helpers+body+test)
  subprocess.run(['cc','-g','-O1','-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],check=True)
  subprocess.run([str(p/'test')]+(['baseline'] if label=='baseline' else []),check=True)
