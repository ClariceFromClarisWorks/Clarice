"""Exercise the actual guest index/catalog routines with host file I/O shims."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[2]
s=(root/'src/commands/repository-client.inc').read_text()
body=s[s.index('typedef struct RepoItem'):s.index('static int repo_marker')]
prefix=r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "src/update/repository.h"
#define PATH_CAP 512
#define REPO_CATALOG "catalog"
#define fsRdPerm 1
#define fsRdWrPerm 3
#define fsFromStart 1
#define fnfErr -43
#define eofErr -39
#define paramErr -50
#define memFullErr -108
#define userCanceledErr -128
static int fail_alloc,cancelled,flushes;
static void *cli_alloc(unsigned long n){return fail_alloc?0:malloc(n);}
static int app_yield(void){return cancelled;}
static int open_path(const char *p,int mode,short *ref){int f=open(p,mode==1?O_RDONLY:O_RDWR);if(f<0)return errno==ENOENT?-43:-1;*ref=f;return 0;}
static int FSRead(short f,long *n,void *p){long want=*n;*n=read(f,p,*n);return *n<0?-1:*n<want?-39:0;}
static int FSClose(short f){return close(f);}
static int SetEOF(short f,long n){return ftruncate(f,n);}
static int SetFPos(short f,int mode,long n){(void)mode;return lseek(f,n,SEEK_SET)<0?-1:0;}
static int io_write(short f,const void *p,long n){return write(f,p,n)==n?0:-1;}
static int checked(const char *p,char *native,char *canonical,int follow){(void)follow;strcpy(native,p);strcpy(canonical,p);return 0;}
static int cli_native_create(const char *p,long type,long creator){(void)type;(void)creator;int f=open(p,O_CREAT|O_EXCL|O_RDWR,0600);return f<0?-1:close(f);}
static int cli_native_open(const char *p,int fork,int write,short *f){(void)fork;return open_path(p,write?3:1,f);}
static int update_flush(const char *p){(void)p;flushes++;return 0;}
static void put(const char *p,const char *s){FILE *f=fopen(p,"w");assert(f);assert(fputs(s,f)>=0);assert(!fclose(f));}
'''
tests=r'''
static void clear(RepoItem *p){while(p){RepoItem *n=p->next;free(p);p=n;}}
int main(void){
 RepoItem item={0};strcpy(item.wanted.name,"ls");RepoRecord a={"ls","",1,36,123,0},b=a;b.version=2;
 assert(!repo_catalog(&item,0)&&!item.known);
 assert(!repo_catalog(0,&a));assert(!repo_catalog(&item,0)&&item.installed.version==1);
 assert(!repo_catalog(0,&b));assert(!repo_catalog(&item,0)&&item.installed.version==2);
 FILE *f=fopen("catalog","a");fputs("tool\tls\t3\t",f);fclose(f);
 assert(!repo_catalog(&item,0)&&item.installed.version==2);b.version=4;assert(!repo_catalog(0,&b));assert(!repo_catalog(&item,0)&&item.installed.version==4);
 put("catalog","CLARICE-INST");assert(!repo_catalog(0,&a));assert(!repo_catalog(&item,0)&&item.installed.version==1);
 put("catalog","");assert(!repo_catalog(0,&a));assert(!repo_catalog(&item,0));
 put("catalog","CLARICE-INSTALLED 1\ninvalid\n");assert(repo_catalog(0,&a));
 const char *valid="CLARICE-REPOSITORY 1\ncore\tClarice\t2\t128\t00000000\tCore.bin\ntool\tls\t2\t36\t00000000\tls.clx\n";
 put("index",valid);RepoItem *list=0;assert(!repo_read_index("index",&list));assert(list&&list->next);clear(list);list=0;
 fail_alloc=1;assert(repo_read_index("index",&list)==memFullErr&&!list);fail_alloc=0;
 cancelled=1;assert(repo_read_index("index",&list)==userCanceledErr);clear(list);list=0;cancelled=0;
 f=fopen("index","a");fputs("tool\tls\t3\t36\t00000000\tls.clx\n",f);fclose(f);assert(repo_read_index("index",&list)==paramErr);clear(list);list=0;
 put("index","CLARICE-REPOSITORY 1\ncore\tClarice\t2\t128\t00000000\tCore.bin");assert(repo_read_index("index",&list)==paramErr);clear(list);
 assert(flushes==5);puts("PASS production catalog append/replay, torn tail/header recovery, malformed record rejection; index duplicates, truncation, allocation failure and cancellation");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(prefix+body+tests)
 subprocess.run(['cc','-fsanitize=address,undefined','-Wno-multichar','-I',str(root),str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],cwd=p,check=True)
