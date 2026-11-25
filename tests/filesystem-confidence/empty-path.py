"""Check empty operands in both the committed and metadata-aware resolvers."""
from pathlib import Path
import subprocess
r=Path.cwd()
s=(r/'src/namespace.inc').read_text()
metadata='static int resolve_full(' in s
begin='static int resolve_full(' if metadata else 'static int resolve_ex('
code=s[s.index(begin):s.index('static int resolve(const char *s,')]
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
static const char *env_get(const char *name){(void)name;return "/home";}
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
extra='assert(cli_resolve_info("",path,canonical,&pb)==bdNamErr);' if metadata else ''
test='int main(void){char path[512],canonical[512];CInfoPBRec pb;assert(resolve_ex("",path,canonical,0)==bdNamErr);assert(resolve_ex("",path,canonical,1)==bdNamErr);'+extra+'assert(!calls);assert(!resolve_ex(".",path,canonical,1));puts("PASS empty paths reject without catalog I/O; explicit dot still resolves");}'
w=r/'work/filesystem-confidence';w.mkdir(parents=True,exist_ok=True)
(w/'empty-path.c').write_text(preamble+code+test)
subprocess.run(['cc','-g','-O1','-fsanitize=address,undefined',str(w/'empty-path.c'),'-o',str(w/'empty-path')],check=True)
subprocess.run([str(w/'empty-path')],check=True)
