from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];w=r/'work/utility-correctness'
stub=r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#define PATH_CAP 512
#define fnfErr -43
#define paramErr -50
#define memFullErr -108
#define bdNamErr -37
#define cli_alloc tracked_alloc
#define cli_free tracked_free
typedef struct {struct {int ioFlAttrib;unsigned long ioFlPyLen,ioFlRPyLen,ioFlLgLen,ioFlRLgLen;struct {unsigned long fdType,fdCreator;} ioFlFndrInfo;} hFileInfo;} CInfoPBRec;
static int output_error,allocs,fail_at,live,polls,cancel_at,printed,errors;
static char output[32768];
static void *tracked_alloc(unsigned long n){if(fail_at&&++allocs==fail_at)return NULL;void *p=malloc(n);if(p)live++;return p;}
static void tracked_free(void *p){if(p){live--;free(p);}}
static void error(const char *s,int e){(void)s;(void)e;errors++;}
static void say(const char *s){assert(strlen(output)+strlen(s)<sizeof output);strcat(output,s);if(!strcmp(s,"\n"))printed++;}
static int app_yield(void){return cancel_at&&++polls>=cancel_at;}
static int resolve_ex(const char *s,char *n,char *c,int f){(void)f;if(!strcmp(s,"missing"))return fnfErr;strcpy(n,s);strcpy(c,s);return 0;}
static int info(const char *s,CInfoPBRec *p){memset(p,0,sizeof *p);if(strstr(s,"file space")){p->hFileInfo.ioFlPyLen=512;p->hFileInfo.ioFlRPyLen=512;}else p->hFileInfo.ioFlAttrib=16;return 0;}
static int isdir(const CInfoPBRec *p){return !!(p->hFileInfo.ioFlAttrib&16);}
static int is_link(const CInfoPBRec *p){(void)p;return 0;}
static int link_target(const char*s,char*t){(void)s;(void)t;return 1;}
static int cli_native_nth(const char *s,int i,char *leaf,CInfoPBRec *p){(void)p;if(i!=1)return fnfErr;int depth=0;for(;*s;s++)if(*s=='/')depth++;strcpy(leaf,depth<24?"d":"file space");return 0;}
static int encode_name(const char *s,char *d,size_t n){if(strlen(s)>=n)return bdNamErr;strcpy(d,s);return 0;}
static int join_path(const char *p,const char *l,char *o){if(strlen(p)+strlen(l)+2>=PATH_CAP)return bdNamErr;snprintf(o,PATH_CAP,"%s/%s",p,l);return 0;}
static void format_size(char *b,unsigned long long n,int h){(void)h;snprintf(b,64,"%llu",n);}
'''
src=(r/'src/commands/inspect.c').read_text().replace('#include "mac-files.h"',stub)
tests=r'''
static void reset(void){assert(!live);output[0]=0;output_error=allocs=polls=printed=errors=fail_at=cancel_at=0;pattern=NULL;kind=all=summarize=human=brief=0;unit=512;maxdepth=INT_MAX;}
int main(void){
 char *find[]={"find","root"};reset();assert(!clarice_main(2,find));assert(printed==26&&!live&&strstr(output,"file space"));
 char *limited[]={"find","root","-maxdepth","12"};reset();assert(!clarice_main(4,limited)&&printed==13&&!live);
 char *du[]={"du","-s","root"};reset();assert(!clarice_main(3,du));assert(!strcmp(output,"2\troot\n")&&!live);
 for(int i=1;i<=26;i++){reset();fail_at=i;assert(clarice_main(2,find)==1&&!live&&errors);}
 reset();cancel_at=15;assert(clarice_main(2,find)==130&&!live);
 reset();char *missing[]={"find","missing"};assert(clarice_main(2,missing)==1&&!live);
 puts("PASS: depth 24, maxdepth 12, both-fork du total, 26 allocation failures, cancellation and missing path; no live frames");
}
'''
p=w/'traversal.c';p.write_text(src+tests)
subprocess.run(['cc','-g','-O1','-fsanitize=address,undefined',str(p),'-o',str(w/'traversal')],check=True)
subprocess.run([str(w/'traversal')],check=True)
