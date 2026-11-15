"""Actual listing loop, hostile HFS output blocks; no emulated HFS timing claims."""
from pathlib import Path
import subprocess,tempfile
s=Path('src/listing.inc').read_text();a=s.index('static int list_impl(');b=s.index('/* Batch bulk paints',a)
source=s[a:b]
preamble=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#define PATH_CAP 256
#define LS_START ((void)0)
#define LS_PHASE(n) ((void)0)
enum {fnfErr=-43,nsvErr=-35,bdNamErr=-37};
typedef struct {unsigned short fdFlags;} FInfo;
typedef struct {unsigned char *ioNamePtr;short ioVRefNum,ioFDirIndex;long ioDirID;unsigned char ioFlAttrib;FInfo ioFlFndrInfo;} HInfo;
typedef union {HInfo hFileInfo;struct {unsigned char *name;short ioVRefNum,index;long ioDrDirID;} dirInfo;} CInfoPBRec;
typedef struct {int detail,human,paged,multi,used,presented;char row[132];} Listing;
static unsigned long list_ticks,list_first,list_entries;
static int output_ref=-1,script_depth,output_error,seen,calls,cancel,fault,oversize;
static short expected_volume;
static char seen_names[8][100];
static unsigned long TickCount(void){return 1;}
static void app_page_begin(void){}
static void term_present(void){}
static int app_yield(void){return cancel;}
static void error(const char *s,int e){(void)s;(void)e;}
static int resolve_ex(const char *s,char *np,char *up,int follow){(void)follow;if(strstr(s,"Missing"))return nsvErr;strcpy(np,s);strcpy(up,s);return 0;}
static int resolve(const char *s,char *np){strcpy(np,s);return 0;}
static void parent_path(char *s){strcpy(s,"/mnt");}
static int info(const char *s,CInfoPBRec *p){memset(p,0,sizeof *p);p->hFileInfo.ioVRefNum=strstr(s,"Test 2")?-2:-1;p->hFileInfo.ioDirID=42;p->hFileInfo.ioFlAttrib=16;return 0;}
static int volume_name(int i,char *raw){if(i>2)return nsvErr;strcpy(raw,i==1?"Boot":"Test 2");return 0;}
static int encode_name(const char *s,char *out,size_t cap){size_t n=0;while(*s){unsigned char c=*s++;if(n+4>=cap)return bdNamErr;if(c=='/'||c=='%'){out[n++]='%';out[n++]='2';out[n++]=c=='/'?'F':'5';}else out[n++]=c;}out[n]=0;return 0;}
static int PBGetCatInfoSync(CInfoPBRec *p){
 const char *names[]={"file","sub",".hidden","a/b%c","1234567890123456789012345678901"};
 calls++;assert(p->hFileInfo.ioVRefNum==expected_volume);assert(p->hFileInfo.ioDirID==42);
 int i=p->hFileInfo.ioFDirIndex;if(fault&&i==3)return -36;if(i>5)return fnfErr;
 unsigned char *n=p->hFileInfo.ioNamePtr;size_t len=strlen(names[i-1]);n[0]=len;memcpy(n+1,names[i-1],len);if(oversize)n[0]=32;
 /* File Manager overwrites directory ID; callers must restore it. */
 p->hFileInfo.ioDirID=800+i;p->hFileInfo.ioFlAttrib=i==2?16:0;
 p->hFileInfo.ioFlFndrInfo.fdFlags=i==1?0x4000:0;return 0;
}
static int list_item(Listing *l,const char *name,const CInfoPBRec *pb){(void)l;(void)pb;strcpy(seen_names[seen++],name);return cancel?130:0;}
static int list_flush(Listing *l){(void)l;return 0;}
'''
main=r'''
static void reset(void){seen=calls=cancel=fault=oversize=0;memset(seen_names,0,sizeof seen_names);}
int main(void){
 for(int v=1;v<=2;v++){const char *path=v==1?"/mnt/Boot/sub":"/mnt/Test 2/sub";expected_volume=-v;reset();assert(!list_impl(path,1,1,0,0,1));assert(seen==7&&calls==6);assert(!strcmp(seen_names[0],"."));assert(!strcmp(seen_names[1],".."));assert(!strcmp(seen_names[5],"a%2Fb%25c"));assert(strlen(seen_names[6])==31);
 reset();assert(!list_impl(path,0,0,0,0,1));assert(seen==3);assert(!strcmp(seen_names[0],"sub"));
 reset();fault=1;assert(list_impl(path,0,1,0,0,1)==1);assert(calls==3);
 reset();oversize=1;assert(list_impl(path,0,1,0,0,1)==1);assert(calls==1);
 reset();cancel=1;assert(list_impl(path,0,0,0,0,1)==130);assert(calls==1);
 }
 reset();assert(list_impl("/mnt/Missing",0,0,0,0,1)==1);assert(!calls&&!seen);
 reset();assert(!list_impl("/mnt",0,0,0,0,1));assert(!calls&&seen==2);assert(!strcmp(seen_names[1],"Test 2"));
 puts("PASS: real listing loop: explicit volumes, overwritten IDs, mixed files/directories, hidden entries, escaped names, 31-byte names, malformed names, EOF, errors, cancellation, missing volumes, virtual mounts");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(preamble+source+main)
 subprocess.run(['cc','-std=c99','-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
