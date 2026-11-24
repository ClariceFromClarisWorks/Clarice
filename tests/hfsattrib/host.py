"""Production attribute parser/native-write boundary, under ASan/UBSan."""
from pathlib import Path
import subprocess,tempfile
s=Path('src/commands/files.c').read_text();code=s[s.index('static void four('):s.index('static int mkdir_one(')]
pre=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define PATH_CAP 512
#define paramErr -50
typedef struct {struct {int ioFlAttrib;struct {unsigned long fdType,fdCreator;unsigned short fdFlags;} ioFlFndrInfo;unsigned char extra[16];} hFileInfo;struct {struct {unsigned short frFlags;} ioDrUsrWds;} dirInfo;} CInfoPBRec;
static CInfoPBRec file,before;static int writes,locks,cancel,native_error;static char output[1024];
static int checked(const char *s,char *n,char *c,int f){(void)f;strcpy(n,s);strcpy(c,s);return 0;}
static int info(const char *s,CInfoPBRec *p){(void)s;*p=file;return 0;}
static int isdir(CInfoPBRec *p){return p->hFileInfo.ioFlAttrib&16;}
static int cli_native_lock(const char *s,int n){(void)s;locks++;if(n)file.hFileInfo.ioFlAttrib|=1;else file.hFileInfo.ioFlAttrib&=~1;return 0;}
static int cli_native_set(const char *s,CInfoPBRec *p){(void)s;writes++;if(native_error)return native_error;file=*p;return 0;}
static int app_yield(void){return cancel;}
static int fail(const char *s,int e){(void)s;(void)e;return 1;}
static void say(const char *s){strcat(output,s);}
static void reset(void){memset(&file,0,sizeof file);file.hFileInfo.ioFlFndrInfo.fdType=0x54455854;file.hFileInfo.ioFlFndrInfo.fdCreator=0x54455354;file.hFileInfo.ioFlFndrInfo.fdFlags=0x2010;memset(file.hFileInfo.extra,0x57,16);before=file;writes=locks=cancel=native_error=0;output[0]=0;}
'''
main=r'''
int main(void){
 char *inspect[]={"hfsattrib","file"};reset();assert(!attr(2,inspect)&&!writes&&strstr(output,"TEXT")&&strstr(output,"TEST"));
 char *v[]={"hfsattrib","file","SIT!:SITx"};reset();assert(!attr(3,v)&&writes==1);assert(file.hFileInfo.ioFlFndrInfo.fdType==0x53495421&&file.hFileInfo.ioFlFndrInfo.fdCreator==0x53495478);assert(file.hFileInfo.ioFlFndrInfo.fdFlags==0x2010&&!memcmp(file.hFileInfo.extra,before.hFileInfo.extra,16));
 const char *bad[]={"SIT:SITx","SIT!:SIT","SIT!!:SITx",":SITx","SIT!:","SIT!:SITxx",":", "SIT!:"};
 for(unsigned i=0;i<sizeof bad/sizeof *bad;i++){reset();v[2]=(char*)bad[i];assert(attr(3,v)==2&&!writes&&!locks&&!memcmp(&file,&before,sizeof file));}
 reset();v[2]="sit!:sitX";assert(!attr(3,v));assert(file.hFileInfo.ioFlFndrInfo.fdType==0x73697421&&file.hFileInfo.ioFlFndrInfo.fdCreator==0x73697458);
 char *options[]={"hfsattrib","--type","BINA","--creator","ABcd","--invisible","on","file"};reset();assert(!attr(8,options)&&writes==1&&file.hFileInfo.ioFlFndrInfo.fdFlags==0x6010);
 char *mixed[]={"hfsattrib","--locked","on","file","SIT!:BAD"};reset();assert(attr(5,mixed)==2&&!writes&&!locks);
 options[4]="BAD";reset();assert(attr(8,options)==2&&!writes&&!locks);
 char *lock[]={"hfsattrib","--locked","on","file"};reset();assert(!attr(4,lock)&&locks==1&&(file.hFileInfo.ioFlAttrib&1));
 reset();file.hFileInfo.ioFlAttrib=16;v[2]="SIT!:SITx";assert(attr(3,v)==1&&!writes);
 reset();native_error=-34;assert(attr(3,v)==1&&!memcmp(&file,&before,sizeof file));
 reset();cancel=1;assert(attr(3,v)==130);
 puts("PASS production hfsattrib inspection/options/shorthand, exact case, malformed input, metadata preservation, native failure and cancellation");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'attrs.c';p.write_text(pre+code+main)
 subprocess.run(['cc','-fsanitize=address,undefined',str(p),'-o',str(p.with_suffix(''))],check=True)
 subprocess.run([str(p.with_suffix(''))],check=True)
