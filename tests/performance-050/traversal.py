"""Actual traversal against deterministic HFS snapshots; host sanitizer/fault checks."""
from pathlib import Path
import subprocess,tempfile
source=Path('src/commands/inspect.c').read_text();source=source[source.index('static char **predicates'):source.index('static void code4')]
pre=r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PATH_CAP 512
enum {memFullErr=-108,bdNamErr=-37,fnfErr=-43,paramErr=-50,nsvErr=-35};
typedef struct {unsigned char *ioNamePtr;short ioVRefNum,ioFDirIndex;long ioDirID;int ioFlAttrib,link;unsigned long ioFlPyLen,ioFlRPyLen;} HInfo;
typedef union {HInfo hFileInfo;struct {unsigned char *name;short volume,index;long ioDrDirID;} dirInfo;} CInfoPBRec;
static int live,attempts,fail_at=-1,cancel_at=-1,polls,fault,output_error,resolves,infos,zero_volume,oversize;
static char output[8192];
static void *cli_alloc(size_t n){if(attempts++==fail_at)return NULL;void *p=malloc(n);if(p)live++;return p;}
static void cli_free(void *p){if(p){live--;free(p);}}
static void error(const char *s,int e){(void)s;(void)e;}
static void say(const char *s){assert(strlen(output)+strlen(s)<sizeof output);strcat(output,s);}
static int app_yield(void){return polls++==cancel_at;}
static int isdir(const CInfoPBRec *p){return !!(p->hFileInfo.ioFlAttrib&16);}
static int is_link(const CInfoPBRec *p){return p->hFileInfo.link;}
static int resolve_ex(const char *p,char *native,char *canonical,int follow){assert(!follow);resolves++;strcpy(canonical,p);if(strstr(p,"Missing"))return -35;strcpy(native,strstr(p,"Test 2")?"Test 2:":"Boot:");return 0;}
static int info(const char *p,CInfoPBRec *pb){infos++;assert(!strcmp(p,"Test 2:")||!strcmp(p,"Boot:"));memset(pb,0,sizeof *pb);pb->hFileInfo.ioFlAttrib=16;pb->hFileInfo.ioDirID=42;pb->hFileInfo.ioVRefNum=zero_volume?0:strstr(p,"Test 2")?-2:-1;return 0;}
static int PBGetCatInfoSync(CInfoPBRec *pb){
 assert(pb->hFileInfo.ioVRefNum==-1||pb->hFileInfo.ioVRefNum==-2);
 assert(pb->hFileInfo.ioDirID==42||pb->hFileInfo.ioDirID==77);
 int index=pb->hFileInfo.ioFDirIndex,sub=pb->hFileInfo.ioDirID==77;
 if(fault&&index==2)return -36;
 const char *names[]={"plain","sub","link","a/b%"};if(index>(sub?1:4))return fnfErr;
 const char *leaf=sub?"nested":names[index-1];unsigned char *name=pb->hFileInfo.ioNamePtr;
 name[0]=strlen(leaf);memcpy(name+1,leaf,name[0]);if(oversize)name[0]=32;
 memset(pb,0,sizeof *pb);pb->hFileInfo.ioFlAttrib=!sub&&index==2?16:0;
 pb->hFileInfo.ioDirID=!sub&&index==2?77:666;pb->hFileInfo.link=!sub&&index==3;
 pb->hFileInfo.ioFlPyLen=512;pb->hFileInfo.ioFlRPyLen=512;return 0;
}
static int encode_name(const char *p,char *q,size_t n){(void)n;while(*p){if(*p=='/'){strcpy(q,"%2F");q+=3;}else if(*p=='%'){strcpy(q,"%25");q+=3;}else *q++=*p;p++;}*q=0;return 0;}
static int join_path(const char *p,const char *l,char *q){if(strlen(p)+strlen(l)+2>=PATH_CAP)return bdNamErr;strcpy(q,p);strcat(q,"/");strcat(q,l);return 0;}
static void format_size(char *b,unsigned long long n,int human){(void)human;snprintf(b,64,"%llu",n);}
'''
main=r'''
static void reset(void){assert(!live);attempts=polls=resolves=infos=output_error=fault=zero_volume=oversize=0;fail_at=cancel_at=-1;output[0]=0;}
int main(void){unsigned long long total;
 for(int v=0;v<2;v++){reset();const char *root=v?"/mnt/Test 2":"/mnt/Boot";assert(!walk(root,0,0,&total));assert(total==4096);assert(resolves==1&&infos==1&&attempts==2);assert(strstr(output,"/sub/nested\n")&&strstr(output,"/a%2Fb%25\n")&&strstr(output,"/link\n"));assert(!live);}
 reset();all=1;assert(!walk("/mnt/Test 2",0,1,&total)&&total==4096&&attempts==2);assert(strstr(output,"2\t/mnt/Test 2/plain\n")&&strstr(output,"2\t/mnt/Test 2/sub/nested\n")&&strstr(output,"8\t/mnt/Test 2\n"));all=0;
 for(int n=0;n<8;n++){reset();fail_at=n;int rc=walk("/mnt/Test 2",0,0,&total);if(n<2)assert(rc==1);else assert(!rc);assert(!live);reset();assert(!walk("/mnt/Test 2",0,1,&total));assert(total==4096);}
 for(int n=0;n<12;n++){reset();cancel_at=n;int rc=walk("/mnt/Test 2",0,0,&total);assert(rc==0||rc==130);assert(!live);}
 reset();fault=1;assert(walk("/mnt/Test 2",0,0,&total)==1);assert(!live);
 reset();assert(walk("/mnt/Missing",0,0,&total)==1);assert(!live&&infos==0);
 reset();zero_volume=1;assert(walk("/mnt/Test 2",0,0,&total)==1);assert(!live);
 reset();oversize=1;assert(walk("/mnt/Test 2",0,0,&total)==1);assert(!live);
 char *patterns[]={"-name","[[:alpha:]]*","-path","*/[!0-9]*"};predicates=patterns;predicate_count=4;
 assert(selected("/root/file9","file9",0,0));assert(!selected("/root/9file","9file",0,0));
 char *escaped[]={"-name","a\\*"};predicates=escaped;predicate_count=2;
 assert(selected("/root/a*","a*",0,0));assert(!selected("/root/abc","abc",0,0));predicate_count=0;
 puts("PASS traversal: two volumes, raw/encoded names, nested directories, links, fork sizes, allocation failures, cancellation, catalog error, missing root, cleanup/recovery");
}
'''
with tempfile.TemporaryDirectory() as d:
 c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(pre+source+main)
 subprocess.run(['cc','-std=c99','-fsanitize=address,undefined',str(c),'src/ports/fnmatch-byte.c','-Isrc/vendor/musl-fnmatch','-Isrc/ports/fnmatch-include','-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
