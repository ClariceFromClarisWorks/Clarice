/* Actual native save implementation, faultable HFS model. Not durability proof. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <setjmp.h>
#define PATH_CAP 512
#define fnfErr -43
#define dupFNErr -48
#define bdNamErr -37
#define permErr -54
#define ioErr -36
#define eofErr -39
#define userCanceledErr -128
#define dskFulErr -34
typedef struct {struct {long ioFlAttrib,ioDirID,ioFlMdDat,ioFlLgLen,ioFlRLgLen,ioFlCrDat,ioFlFndrInfo,ioFlXFndrInfo;} hFileInfo;} CInfoPBRec;
static struct File {char path[512];char forks[2][8192];long len[2];CInfoPBRec meta;long type,creator;} files[8];
static struct Ref {int file,fork;long pos;} refs[16];
static int operation,fail_at,fail_again,short_at,crash_at,cancel,allocated,readonly,fail_code=ioErr;
static jmp_buf crash;
static int fault(void){operation++;if(operation==crash_at)longjmp(crash,1);return (operation==fail_at||operation==fail_again)?fail_code:0;}
static int find(const char *p){for(int i=0;i<8;i++)if(*files[i].path&&!strcmp(files[i].path,p))return i;return -1;}
static void *cli_alloc(unsigned long n){void *p=malloc(n);if(p)allocated++;return p;}
static void cli_free(void *p){allocated--;free(p);}
static void errsay(const char *p){(void)p;}
static int port_errno(int e){errno=e==dskFulErr?ENOSPC:EIO;return -1;}
static int resolve(const char *p,char *out){strcpy(out,p);return 0;}
static int info(const char *p,CInfoPBRec *out){int e=fault();if(e)return e;int i=find(p);if(i<0)return fnfErr;*out=files[i].meta;out->hFileInfo.ioFlLgLen=files[i].len[0];out->hFileInfo.ioFlRLgLen=files[i].len[1];return 0;}
static int cli_native_volume(const char *p,short *v){(void)p;*v=1;return 0;}
static int FlushVol(void *p,short v){(void)p;(void)v;return fault();}
static int cli_native_create(const char *p,long t,long c){int e=fault();if(e)return e;if(readonly)return permErr;if(find(p)>=0)return dupFNErr;for(int i=0;i<8;i++)if(!*files[i].path){memset(&files[i],0,sizeof files[i]);strcpy(files[i].path,p);files[i].type=t;files[i].creator=c;files[i].meta.hFileInfo.ioDirID=i+10;return 0;}return dskFulErr;}
static int cli_native_delete(const char *p){int e=fault();if(e)return e;int i=find(p);if(i<0)return fnfErr;files[i].path[0]=0;return 0;}
static int cli_native_rename(const char *p,const char *q){int e=fault();if(e)return e;int i=find(p);if(i<0)return fnfErr;if(find(q)>=0)return dupFNErr;strcpy(files[i].path,q);return 0;}
static int cli_native_set(const char *p,CInfoPBRec *m){int e=fault();if(e)return e;int i=find(p);if(i<0)return fnfErr;files[i].meta=*m;return 0;}
static int cli_native_open(const char *p,int fork,int write,short *r){int e=fault();if(e)return e;if(write&&readonly)return permErr;int i=find(p);if(i<0)return fnfErr;for(int j=1;j<16;j++)if(!refs[j].file){refs[j]=(struct Ref){i+1,fork,0};*r=j;return 0;}return ioErr;}
static int cli_native_close(short r){assert(refs[r].file);refs[r].file=0;return fault();}
static int cli_native_read(short r,void *b,long *n){int e=fault();if(e)return e;struct Ref *ref=&refs[r];struct File *f=&files[ref->file-1];long wanted=*n;if(*n>f->len[ref->fork]-ref->pos)*n=f->len[ref->fork]-ref->pos;memcpy(b,f->forks[ref->fork]+ref->pos,*n);ref->pos+=*n;return *n<wanted?eofErr:0;}
static int cli_native_write(short r,const void *b,long *n){int e=fault();if(e)return e;struct Ref *ref=&refs[r];struct File *f=&files[ref->file-1];if(operation==short_at&&*n>0)(*n)--;assert(ref->pos+*n<=8192);memcpy(f->forks[ref->fork]+ref->pos,b,*n);ref->pos+=*n;f->len[ref->fork]=ref->pos;return 0;}
static int app_yield(void){return cancel;}
#include "../../src/safe-save.inc"
static const char *dest="Disk With Spaces:Docs:note";
static void reset(int existing){assert(!allocated&&!active_save);memset(files,0,sizeof files);memset(refs,0,sizeof refs);operation=fail_at=fail_again=short_at=crash_at=cancel=readonly=0;if(existing){assert(!cli_native_create(dest,0,0));int i=find(dest);memcpy(files[i].forks[0],"ORIGINAL",8);files[i].len[0]=8;memset(files[i].forks[1],0x9a,5000);files[i].len[1]=5000;files[i].meta.hFileInfo.ioFlFndrInfo=123;files[i].meta.hFileInfo.ioFlXFndrInfo=456;files[i].meta.hFileInfo.ioFlCrDat=789;}operation=0;}
static int exact(int i,int original){return i>=0&&files[i].len[0]==8&&!memcmp(files[i].forks[0],original?"ORIGINAL":"REPLACED",8);}
static void preserved(void){int old=0,new=0;for(int i=0;i<8;i++){if(!*files[i].path)continue;if(exact(i,1)||exact(i,0)){assert(files[i].len[1]==5000);for(int j=0;j<5000;j++)assert((unsigned char)files[i].forks[1][j]==0x9a);assert(files[i].meta.hFileInfo.ioFlFndrInfo==123&&files[i].meta.hFileInfo.ioFlXFndrInfo==456&&files[i].meta.hFileInfo.ioFlCrDat==789);if(exact(i,1))old=1;else new=1;}}assert(old||new);}
static int run(void){char path[520];void *s=cli_replace_begin(dest,path,sizeof path);if(!s)return -1;short r=-1;int e=cli_native_open(path+7,0,1,&r);if(!e){long n=8;e=cli_native_write(r,"REPLACED",&n);if(!e&&n!=8)e=ioErr;int c=cli_native_close(r);if(!e)e=c;}if(e){cli_replace_abort(s);return -1;}return cli_replace_commit(s);}
int main(void){
 reset(1);assert(!run());int count=operation,i=find(dest);assert(exact(i,0)&&files[i].len[1]==5000&&files[i].forks[1][4999]==(char)0x9a);assert(files[i].meta.hFileInfo.ioFlFndrInfo==123&&files[i].meta.hFileInfo.ioFlXFndrInfo==456&&files[i].meta.hFileInfo.ioFlCrDat==789);assert(!allocated);
 for(int n=1;n<=count;n++){reset(1);fail_at=n;run();preserved();assert(!allocated);for(int j=1;j<16;j++)assert(!refs[j].file);}
 for(int n=1;n<=count;n++){reset(1);short_at=n;run();preserved();assert(!allocated);}
 for(int n=1;n<=count;n++){reset(1);crash_at=n;if(!setjmp(crash))run();preserved();/* Simulated process death: heap/ref state is lost; filesystem remains. */if(active_save){free(active_save);active_save=0;allocated=0;}memset(refs,0,sizeof refs);}
 for(int n=1;n<=count;n++){reset(1);fail_at=n;fail_code=dskFulErr;run();preserved();assert(!allocated);}fail_code=ioErr;
 /* Two independent faults exercise failed rollback; all original bytes retained. */
 for(int n=1;n<=count;n++)for(int m=n+1;m<=count+3;m++){reset(1);fail_at=n;fail_again=m;run();preserved();assert(!allocated);
  if(find(dest)<0){fail_at=fail_again=0;int old=-1;for(int j=0;j<8;j++)if(*files[j].path&&exact(j,1))old=j;assert(old>=0);char recovery[512];strcpy(recovery,files[old].path);assert(!cli_native_rename(recovery,dest));assert(exact(find(dest),1));}
}
 reset(0);assert(!run()&&exact(find(dest),0));assert(files[find(dest)].type=='TEXT'&&files[find(dest)].creator=='CLRC');
 int new_count=operation;for(int n=1;n<=new_count;n++){reset(0);fail_at=n;run();int f=find(dest);assert(f<0||exact(f,0));assert(!allocated);}
 reset(1);char staged[520];void *pending=cli_replace_begin(dest,staged,sizeof staged);assert(pending);char oldname[512];strcpy(oldname,((SafeSave*)pending)->old);cli_replace_abort(pending);
 assert(!cli_native_create(oldname,0,0));assert(!cli_replace_begin(dest,staged,sizeof staged));assert(exact(find(dest),1)&&find(oldname)>=0&&!allocated);

 reset(1);cancel=1;assert(run()<0&&exact(find(dest),1));
 reset(1);readonly=1;assert(run()<0&&exact(find(dest),1));
 reset(1);files[find(dest)].meta.hFileInfo.ioFlAttrib=1;assert(run()<0&&exact(find(dest),1));
 reset(1);for(int n=0;n<20;n++){assert(!run());assert(!allocated);int total=0;for(int j=0;j<8;j++)total+=!!files[j].path[0];assert(total==1);}
 printf("PASS: success/new/metadata, %d fault points, %d interruption points, short-write, disk-full and paired faults/rollback, new-file faults, stale recovery collision, cancellation, read-only, locked, 20 repeated saves; no retained allocations\n",count,count);
}
