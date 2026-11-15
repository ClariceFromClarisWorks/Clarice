"""Compile production command selection against bounded file/namespace mocks."""
from pathlib import Path
import subprocess
s=Path('src/shell.c').read_text()
def function(name):
 start=s.index('static int '+name+'(');p=s.index('{',start);depth=1;q=p+1
 while depth:
  depth+=(s[q]=='{')-(s[q]=='}');q+=1
 return s[start:q]
source=r'''
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#define PATH_CAP 512
#define fsRdPerm 1
#define eofErr -39
#define dispatch_legacy 0
typedef struct {struct {int ioFlAttrib;} hFileInfo;} CInfoPBRec;
struct File {const char *path,*data;int dir;} files[]={
 {"/bin/ls","CLX1xxx",0},{"/bin/run.sh","echo ok\n",0},
 {"/bin/script","#!/bin/sh\necho ok\n",0},{"/bin/tool","bad",0},
 {"/usr/bin/tool","CLX1xxx",0},{"/bin/folder","",1},
 {"/bin/bad","not executable",0},{"/bin/binary.sh","a\0b",0},
 {"/bin/wrong","#!/bin/python\n",0},{"/bin/short","#!/bin/sh",0},
 {"/mnt/Test Disk/ls","CLX1xxx",0}};
static int resolves,opens,closes;
static const char *path_env="/bin:/usr/bin";
static int resolve(const char *p,char *n){resolves++;if(!strcmp(p,"/bin/link"))p="/bin/ls";snprintf(n,PATH_CAP,"Disk:%s",p);return 0;}
static int lookup(const char *n){if(strncmp(n,"Disk:",5))return -1;for(int i=0;i<11;i++)if(!strcmp(n+5,files[i].path))return i;return -1;}
static int info(const char *n,CInfoPBRec *p){int i=lookup(n);if(i<0)return -43;p->hFileInfo.ioFlAttrib=files[i].dir?16:0;return 0;}
static void pstr(unsigned char *n,const char *p){n[0]=strlen(p);memcpy(n+1,p,n[0]);}
static int HOpen(int a,int b,const unsigned char *pn,int mode,short *ref){(void)a;(void)b;(void)mode;char n[256];memcpy(n,pn+1,pn[0]);n[pn[0]]=0;int i=lookup(n);if(i<0)return -43;opens++;*ref=i;return 0;}
static int FSRead(short ref,long *count,char *out){size_t n=ref==7?3:strlen(files[ref].data);if(n>(size_t)*count)n=*count;memcpy(out,files[ref].data,n);*count=n;return eofErr;}
static void FSClose(short ref){(void)ref;closes++;}
static void errsay(const char *s){(void)s;}
static const char *env_get(const char *k){assert(!strcmp(k,"PATH"));return path_env;}
'''
source+='\n'.join(function(n) for n in ['program_kind_native','program_kind','path_entry','command_path'])
source+=r'''
static void check(const char *name,int want,int kindwant,const char *path){char out[PATH_CAP];int kind=0;resolves=opens=closes=0;int got=command_path(name,out,&kind);assert(got==want);if(got==1){assert(kind==kindwant);assert(!strcmp(out,path));}assert(opens==closes);}
int main(void){
 check("ls",1,2,"/bin/ls");assert(resolves==1&&opens==1);
 check("/bin/ls",1,2,"/bin/ls");assert(resolves==1&&opens==1);
 check("link",1,2,"/bin/link");
 check("run.sh",1,1,"/bin/run.sh");check("script",1,1,"/bin/script");
 check("tool",1,2,"/usr/bin/tool");check("folder",-1,0,"");
 check("bad",-1,0,"");check("binary.sh",-1,0,"");
 check("wrong",-1,0,"");check("short",-1,0,"");check("absent",0,0,"");
 check("/mnt/Test Disk/ls",1,2,"/mnt/Test Disk/ls");
 char out[PATH_CAP];assert(command_path("ls",out,0)==1);assert(program_kind("/bin/ls")==2);
 puts("PASS: 15 command selection/type checks; one resolve/open for first PATH match; descriptors closed");
}
'''
p=Path('work/list-drag/dispatch-host.c');p.parent.mkdir(parents=True,exist_ok=True);p.write_text(source)
subprocess.run(['cc','-fsanitize=address,undefined','-g',str(p),'-o','work/list-drag/dispatch-host'],check=True)
subprocess.run(['work/list-drag/dispatch-host'],check=True)
