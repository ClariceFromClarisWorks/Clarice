from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];w=r/'work/utility-correctness';w.mkdir(exist_ok=True)
s=(r/'src/commands/files.c').read_text()
parts=s[s.index('typedef struct RemoveFrame'):s.index('static int temporary_sibling')]+s[s.index('typedef struct CopyFrame'):s.index('static int transfer')]
stub=r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#define PATH_CAP 512
static void file_progress_current(const char *s){(void)s;}
static void file_progress_show(int done){(void)done;}
static void file_progress_complete(void){}
#define bdNamErr -37
#define fnfErr -43
#define dirNFErr -120
#define paramErr -50
#define memFullErr -108
#define nsvErr -35
static int recursive=1,force,live,allocs,fail_at,polls,cancel_at,copy_fail;
typedef struct {int dir;struct {short ioVRefNum;} hFileInfo;struct {long ioDrDirID;} dirInfo;} CInfoPBRec;
static char directories[1024][PATH_CAP];static int directory_count,enum_calls,enum_fail;
static int app_yield(void){return cancel_at&&++polls>=cancel_at;}
static void *cli_alloc(size_t n){if(fail_at&&++allocs==fail_at)return NULL;void*p=malloc(n);if(p)live++;return p;}
static void cli_free(void*p){if(p){live--;free(p);}}
static int checked(const char*p,char*n,char*c,int x){(void)x;if(strlen(p)>=PATH_CAP)return bdNamErr;strcpy(n,p);strcpy(c,p);return 0;}
static int resolve_ex(const char*p,char*n,char*c,int x){return checked(p,n,c,x);}
static int info(const char*p,CInfoPBRec*b){struct stat st;if(lstat(p,&st))return fnfErr;b->dir=S_ISDIR(st.st_mode);b->hFileInfo.ioVRefNum=-1;if(b->dir){int i;for(i=0;i<directory_count;i++)if(!strcmp(directories[i],p))break;if(i==directory_count){assert(i<1024);strcpy(directories[directory_count++],p);}b->dirInfo.ioDrDirID=i;}return 0;}
static int isdir(CInfoPBRec*b){return b->dir;}
static int same_name(const char*a,const char*b){return !strcmp(a,b);}
static int cli_native_delete(const char*p){CInfoPBRec b;if(info(p,&b))return fnfErr;return b.dir?rmdir(p):unlink(p);}
static int cli_native_mkdir(const char*p){return mkdir(p,0700);}
static int cli_native_nth(const char*p,int index,char*leaf,CInfoPBRec*b){(void)b;DIR*d=opendir(p);if(!d)return -1;struct dirent*e;while((e=readdir(d)))if(strcmp(e->d_name,".")&&strcmp(e->d_name,"..")&&!--index){strcpy(leaf,e->d_name);closedir(d);return 0;}closedir(d);return fnfErr;}
static int directory_nth(short volume,long id,int index,char*leaf,CInfoPBRec*b){assert(volume==-1&&id>=0&&id<directory_count);if(enum_fail&&++enum_calls==enum_fail)return -36;return cli_native_nth(directories[id],index,leaf,b);}
static int encode_name(const char*p,char*q,size_t n){if(strlen(p)>=n)return bdNamErr;strcpy(q,p);return 0;}
static int join_path(const char*p,const char*l,char*q){return snprintf(q,PATH_CAP,"%s/%s",p,l)>=PATH_CAP?bdNamErr:0;}
static int copy_file(const char*s,const char*d,CInfoPBRec*b){(void)b;if(copy_fail)return -36;FILE*i=fopen(s,"rb"),*o=fopen(d,"wb");assert(i&&o);int c;while((c=fgetc(i))!=EOF)fputc(c,o);fclose(i);fclose(o);return 0;}
'''
tests=r'''
int main(void){char base[]="/tmp/clarice-depth-XXXXXX";assert(mkdtemp(base));assert(!chdir(base));mkdir("source",0700);char path[512]="source";for(int i=0;i<24;i++){strcat(path,"/d");assert(!mkdir(path,0700));}strcat(path,"/file space");FILE*f=fopen(path,"wb");fputs("data",f);fclose(f);
 assert(!copy_one("source","copy",0)&&!live);assert(!remove_one("copy",0,0)&&!live);assert(access("copy",F_OK));
 for(int i=1;i<=25;i++){allocs=0;fail_at=i;assert(copy_one("source","copy",0)==memFullErr&&!live);fail_at=0;force=1;assert(!remove_one("copy",0,0)&&!live);force=0;}
 assert(!copy_one("source","copy",0));fail_at=12;allocs=0;assert(remove_one("copy",0,0)==memFullErr&&!live);fail_at=0;assert(!remove_one("copy",0,0));
 assert(!copy_one("source","copy",0));cancel_at=12;polls=0;assert(remove_one("copy",0,0)==130&&!live);cancel_at=0;assert(!remove_one("copy",0,0));
 cancel_at=12;polls=0;assert(copy_one("source","copy",0)==130&&!live);cancel_at=0;assert(!remove_one("copy",0,0));
 enum_calls=0;enum_fail=5;assert(copy_one("source","copy",0)==-36&&!live);enum_fail=0;assert(!remove_one("copy",0,0));
 assert(!copy_one("source","copy",0));enum_calls=0;enum_fail=5;assert(remove_one("copy",0,0)==-36&&!live);enum_fail=0;assert(!remove_one("copy",0,0));
 copy_fail=1;assert(copy_one("source","copy",0)==-36&&!live);copy_fail=0;assert(!remove_one("copy",0,0));
 assert(!remove_one("source",0,0)&&!live);assert(!chdir("/"));assert(!rmdir(base));puts("PASS deep cp/rm, 25 allocation boundaries, cancellation, copy failure and cleanup");}
'''
(w/'file-depth.c').write_text(stub+parts+tests)
subprocess.run(['cc','-g','-O1','-fsanitize=address,undefined',str(w/'file-depth.c'),'-o',str(w/'file-depth')],check=True)
subprocess.run([str(w/'file-depth')],check=True)
