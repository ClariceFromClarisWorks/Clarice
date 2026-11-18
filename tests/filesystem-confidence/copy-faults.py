"""Exercise actual fork-copy/replacement code, failing each native boundary once."""
from pathlib import Path
import subprocess
r=Path.cwd();w=r/'work/filesystem-confidence';w.mkdir(parents=True,exist_ok=True)
s=(r/'src/commands/files.c').read_text();code=s[s.index('static int temporary_sibling'):s.index('typedef struct CopyFrame')]
pre=r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#define PATH_CAP 512
enum{bdNamErr=-37,dupFNErr=-48,fnfErr=-43,paramErr=-50,fLckdErr=-45,eofErr=-39,ioErr=-36};
typedef struct {unsigned long fdType,fdCreator;unsigned short fdFlags;} Finder;
typedef struct {struct{int ioFlAttrib,ioDirID;Finder ioFlFndrInfo;unsigned long ioFlCrDat,ioFlMdDat;}hFileInfo;}CInfoPBRec;
static int preserve=1;static unsigned long sequence;
typedef struct {char name[512];unsigned char data[2][9000];long len[2];CInfoPBRec meta;int used;} File;
static File fs[8];static struct{int file,fork;long pos;int open;}refs[32];
static int count,fail_at,short_at,cancel_at,polls;static char diagnostic[2048];
static int boundary(void){return ++count==fail_at?ioErr:0;}
static int find(const char*p){for(int i=0;i<8;i++)if(fs[i].used&&!strcmp(fs[i].name,p))return i;return -1;}
static int isdir(CInfoPBRec*p){return !!(p->hFileInfo.ioFlAttrib&16);}
static unsigned long cli_ticks(void){return 123;}
static int app_yield(void){return ++polls==cancel_at;}
static void errsay(const char*p){strcat(diagnostic,p);}
static int info(const char*p,CInfoPBRec*b){int e=boundary();if(e)return e;int i=find(p);if(i<0)return fnfErr;*b=fs[i].meta;return 0;}
static int cli_native_create(const char*p,unsigned long a,unsigned long b){(void)a;(void)b;int e=boundary();if(e)return e;if(find(p)>=0)return dupFNErr;for(int i=0;i<8;i++)if(!fs[i].used){memset(&fs[i],0,sizeof fs[i]);fs[i].used=1;strcpy(fs[i].name,p);fs[i].meta.hFileInfo.ioDirID=i+1;return 0;}abort();}
static int cli_native_delete(const char*p){int e=boundary();if(e)return e;int i=find(p);if(i<0)return fnfErr;fs[i].used=0;return 0;}
static int cli_native_rename(const char*a,const char*b){int e=boundary();if(e)return e;int i=find(a);if(i<0)return fnfErr;if(find(b)>=0)return dupFNErr;strcpy(fs[i].name,b);return 0;}
static int cli_native_open(const char*p,int fork,int write,short*out){(void)write;int e=boundary();if(e)return e;int i=find(p);if(i<0)return fnfErr;for(int j=0;j<32;j++)if(!refs[j].open){refs[j].open=1;refs[j].file=i;refs[j].fork=fork;refs[j].pos=0;*out=j;return 0;}abort();}
static int cli_native_read(short ref,void*p,long*n){int e=boundary();if(e)return e;assert(refs[ref].open);File*f=&fs[refs[ref].file];long left=f->len[refs[ref].fork]-refs[ref].pos;if(*n>left){*n=left;e=eofErr;}memcpy(p,f->data[refs[ref].fork]+refs[ref].pos,*n);refs[ref].pos+=*n;return e;}
static int cli_native_write(short ref,const void*p,long*n){int e=boundary();if(e)return e;assert(refs[ref].open);if(count==short_at&&*n)--*n;File*f=&fs[refs[ref].file];assert(refs[ref].pos+*n<=9000);memcpy(f->data[refs[ref].fork]+refs[ref].pos,p,*n);refs[ref].pos+=*n;f->len[refs[ref].fork]=refs[ref].pos;return 0;}
static int cli_native_close(short ref){assert(refs[ref].open);refs[ref].open=0;return boundary();}
static int cli_native_set(const char*p,CInfoPBRec*b){int e=boundary();if(e)return e;int i=find(p);assert(i>=0);fs[i].meta=*b;return 0;}
static int cli_native_lock(const char*p,int on){int e=boundary();if(e)return e;int i=find(p);assert(i>=0);fs[i].meta.hFileInfo.ioFlAttrib=on;return 0;}
'''
test=r'''
static File original,old;
static void reset(int exists){memset(fs,0,sizeof fs);memset(refs,0,sizeof refs);count=fail_at=short_at=cancel_at=polls=0;sequence=0;*diagnostic=0;fs[0]=original;if(exists)fs[1]=old;}
static int equal(File*a,File*b){return a->len[0]==b->len[0]&&a->len[1]==b->len[1]&&!memcmp(a->data[0],b->data[0],a->len[0])&&!memcmp(a->data[1],b->data[1],a->len[1]);}
static void verify(int rc,int exists){assert(equal(&fs[0],&original)&&!strcmp(fs[0].name,"A:src"));for(int j=0;j<32;j++)assert(!refs[j].open);int d=find("A:dst");if(d>=0)assert(equal(&fs[d],&original)|| (exists&&equal(&fs[d],&old)));else assert(rc);if(!rc){assert(d>=0&&equal(&fs[d],&original));assert(fs[d].meta.hFileInfo.ioFlFndrInfo.fdType==original.meta.hFileInfo.ioFlFndrInfo.fdType);assert(fs[d].meta.hFileInfo.ioFlMdDat==original.meta.hFileInfo.ioFlMdDat);}if(exists){int preserved=0;for(int i=1;i<8;i++)if(fs[i].used&&(equal(&fs[i],&old)||equal(&fs[i],&original)))preserved=1;assert(preserved);}}
int main(void){strcpy(original.name,"A:src");original.used=1;original.meta.hFileInfo.ioDirID=1;original.meta.hFileInfo.ioFlAttrib=1;original.meta.hFileInfo.ioFlFndrInfo.fdType=0x54455854;original.meta.hFileInfo.ioFlMdDat=1234;original.len[0]=8301;original.len[1]=4353;for(int f=0;f<2;f++)for(int i=0;i<original.len[f];i++)original.data[f][i]=(i*37+f)%256;old=original;strcpy(old.name,"A:dst");old.meta.hFileInfo.ioDirID=2;old.meta.hFileInfo.ioFlAttrib=0;memset(old.data,0xEE,sizeof old.data);int cases=0;
 for(int exists=0;exists<2;exists++){reset(exists);assert(!copy_file("A:src","A:dst",&original.meta));int boundaries=count,yields=polls;verify(0,exists);cases++;
 for(int n=1;n<=boundaries;n++){reset(exists);fail_at=n;int rc=copy_file("A:src","A:dst",&original.meta);assert(rc);verify(rc,exists);cases++;}
 for(int n=1;n<=boundaries;n++){reset(exists);short_at=n;int rc=copy_file("A:src","A:dst",&original.meta);verify(rc,exists);cases++;}
 for(int n=1;n<=yields;n++){reset(exists);cancel_at=n;int rc=copy_file("A:src","A:dst",&original.meta);assert(rc==130);verify(rc,exists);assert(exists?equal(&fs[find("A:dst")],&old):find("A:dst")==-1);cases++;}
 }printf("PASS %d fork-copy/replacement cases: each native boundary, short writes, cancellation, source integrity, complete destination/recovery and closed refs\n",cases);}
'''
(w/'copy-faults.c').write_text(pre+code+test)
subprocess.run(['cc','-g','-O1','-fsanitize=address,undefined',str(w/'copy-faults.c'),'-o',str(w/'copy-faults')],check=True)
subprocess.run([str(w/'copy-faults')],check=True)
