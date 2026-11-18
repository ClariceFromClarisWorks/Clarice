"""Exercise production resolution with same-operation catalog result reuse."""
from pathlib import Path
import tempfile, subprocess
s=Path('src/namespace.inc').read_text()
code=s[s.index('static int resolve_full('):s.index('static int resolve(const char *s,')]
preamble=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#define PATH_CAP 512
#define bdNamErr -37
#define fnfErr -43
#define dirNFErr -120
#define paramErr -50
typedef struct {struct {int ioFlAttrib;} hFileInfo;int identity;} CInfoPBRec;
static char unix_cwd[512]="/tmp";
struct Node {const char *path,*target;int directory;} nodes[]={
 {"/",0,1},{"/tmp",0,1},{"/tmp/file",0,0},{"/mnt",0,1},
 {"/mnt/Test 2",0,1},{"/mnt/Test 2/file",0,0},{"/alias","/tmp",0},
 {"/tmp/ref","file",0},{"/tmp/up","..",0},{"/cycle","/cycle",0}};
static int calls,fail;
static int native_path(const char *s,char *p){strcpy(p,s);return 0;}
static int unix_from_native(const char *s,char *p){strcpy(p,s);return 0;}
static void parent_path(char *s){char *p=strrchr(s,'/');if(p==s)s[1]=0;else if(p)*p=0;}
static int info(const char *s,CInfoPBRec *p){calls++;if(fail)return -36;
 for(unsigned i=0;i<sizeof nodes/sizeof *nodes;i++)if(!strcasecmp(s,nodes[i].path)){
 p->identity=i;p->hFileInfo.ioFlAttrib=nodes[i].directory?16:0;return 0;}return fnfErr;}
static int is_link(const CInfoPBRec *p){return nodes[p->identity].target!=0;}
static int link_target(const char *s,char *p){CInfoPBRec info_;int e=info(s,&info_);if(e)return e;strcpy(p,nodes[info_.identity].target);return 0;}
'''
main=r'''
int main(void){
 const char *cases[]={"/",".","..","file","/tmp/file","/tmp/.","/tmp/..",
 "/alias/file","/tmp/ref","/tmp/up","/alias/../tmp/file",
 "/mnt/Test 2/file","/mnt/test 2/../Test 2/file","native:/tmp/file",
 "~","~/tmp/file","/missing","/tmp/missing","/tmp/file/..","/cycle"};
 for(unsigned i=0;i<sizeof cases/sizeof *cases;i++){
  char a[512],b[512],ca[512],cb[512];CInfoPBRec pa,pb;
  calls=0;int ea=resolve_ex(cases[i],a,ca,1);if(!ea)ea=info(a,&pa);int before=calls;
  calls=0;int eb=cli_resolve_info(cases[i],b,cb,&pb);assert(ea==eb);
  if(!ea){assert(!strcmp(a,b)&&!strcmp(ca,cb));assert(pa.identity==pb.identity);assert(calls<=before);}
 }
 char path[512];CInfoPBRec pb;
 calls=0;assert(!cli_resolve_info("/tmp/file",path,0,&pb));assert(calls==2);
 assert(!cli_resolve_info("/mnt/Test 2/file",path,0,&pb));assert(pb.identity==5);
 assert(!cli_resolve_info("/tmp/file",path,0,&pb));assert(pb.identity==2);
 fail=1;pb.identity=-1;assert(cli_resolve_info("/tmp/file",path,0,&pb)==-36);assert(pb.identity==-1);
 assert(cli_resolve_info("/tmp/file",path,0,0)==paramErr);
 puts("PASS same-operation metadata: paths, links, dot/dotdot, volumes/spaces, missing/error, no stale result; 3->2 catalog calls");
}
'''
with tempfile.TemporaryDirectory() as d:
    c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(preamble+code+main)
    subprocess.run(['cc','-std=c99','-fsanitize=address,undefined',str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
