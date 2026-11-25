"""Production native glob/write boundaries, with deterministic File Manager shims."""
from pathlib import Path
import subprocess,tempfile

def function(s,name):
 a=s.index('static ',s.rfind('\n',0,s.index(name+'('))+1);b=s.index('{',a);n=1;e=b+1
 while n:
  n+=(s[e]=='{')-(s[e]=='}');e+=1
 return s[a:e]
pre=r'''
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define PATH_CAP 512
#define fnfErr -43
#define PORT_APPEND_REF 131072
#define fsFromLEOF 2
typedef char *Ptr;
typedef struct {struct {short ioRefNum;Ptr ioBuffer;long ioReqCount,ioActCount;short ioPosMode;}ioParam;}ParamBlockRec;
static char data[32]="original";static long pos=0,len=8;static int failure;
static int vstream_code(int r){return 0;}
static long vstream_write(int r,unsigned long n){return -1;}
static int port_errno(int e){return -1;}
static int FSWrite(short r,long*n,const void*p){assert(r==7);if(failure)return failure;memcpy(data+pos,p,*n);pos+=*n;if(pos>len)len=pos;return 0;}
static int PBWriteSync(ParamBlockRec*p){assert(p->ioParam.ioPosMode==fsFromLEOF);if(failure)return failure;pos=len;p->ioParam.ioActCount=p->ioParam.ioReqCount;return FSWrite(p->ioParam.ioRefNum,&p->ioParam.ioActCount,p->ioParam.ioBuffer);}
typedef struct{struct {unsigned char*ioNamePtr;short ioVRefNum;long ioDirID;int ioFDirIndex;}hFileInfo;}CInfoPBRec;
static int PBGetCatInfoSync(CInfoPBRec*p){if(p->hFileInfo.ioFDirIndex>1)return fnfErr;p->hFileInfo.ioNamePtr[0]=31;memset(p->hFileInfo.ioNamePtr+1,'%',31);return 0;}
static int directory(const char*p,short*v,long*i){*v=1;*i=2;return 0;}
static int sh_fail(const char*p){return 2;}
static int app_yield(void){return 0;}
struct sh_args {char*v[4];int n;};
static int sh_arg(struct sh_args*a,const char*p){a->v[a->n++]=strdup(p);return 0;}
'''
s=Path('src/sh.inc').read_text();code=pre+function(Path('src/port-files.inc').read_text(),'cli_file_write')+function(s,'sh_pattern')+function(s,'sh_glob')+r'''
int main(void){
 assert(cli_file_write(7|PORT_APPEND_REF,"A",1)==1&&!memcmp(data,"originalA",9));
 pos=0;assert(cli_file_write(7|PORT_APPEND_REF,"B",1)==1&&!memcmp(data,"originalAB",10));
 pos=0;failure=-34;assert(cli_file_write(7|PORT_APPEND_REF,"X",1)==-1&&len==10);failure=0;
 assert(cli_file_write(7,"Z",1)==1&&data[0]=='Z');
 char path[512];unsigned char mode[512]={0};struct sh_args a={0};
 memset(path,'a',450);path[450]='/';path[451]='*';path[452]=0;
 assert(sh_glob(&a,path,mode)==2&&!a.n);
 strcpy(path,"dir/*");assert(!sh_glob(&a,path,mode)&&a.n==1&&strlen(a.v[0])==97);free(a.v[0]);
 puts("PASS encoded HFS glob bounds; append after seek, native write failure, ordinary overwrite");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'test.c';p.write_text(code);subprocess.run(['cc','-g','-fsanitize=address,undefined',str(p),'-o',d+'/test'],check=True);subprocess.run([d+'/test'],check=True)
