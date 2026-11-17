"""Actual native rename helper: never combine one volume's ID with another volume."""
from pathlib import Path
import subprocess
import tempfile


def function(source, name):
    start = source.index('int ' + name + '(')
    if source[max(0, start - 7):start] == 'static ':
        start -= 7
    brace = source.index('{', start)
    depth, end = 1, brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


ns = Path('src/namespace.inc').read_text()
shell = Path('src/shell.c').read_text()
helpers = '\n'.join(function(ns, n) for n in ['same_name', 'volume_lookup', 'cli_native_volume'])
helpers += '\n' + function(shell, 'info')
rename = function(ns, 'shell_rename') + '\n' + function(ns, 'shell_flush_native')
preamble = r'''
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
enum {nsvErr=-35,dupFNErr=-48,bdNamErr=-37,fnfErr=-43};
typedef char *Ptr;
typedef struct {unsigned char *ioNamePtr;short ioVolIndex,ioVRefNum;} Volume;
typedef struct {short ioVRefNum;long ioDirID;unsigned char *ioNamePtr;} File;
typedef struct {Volume volumeParam;File fileParam;struct {Ptr ioMisc;} ioParam;} HParamBlockRec;
typedef struct {unsigned char *ioNamePtr;short ioVRefNum,ioFDirIndex;long ioFlParID;} Catalog;
typedef struct {Catalog hFileInfo;} CInfoPBRec;
static int duplicate, fault, rename_calls, flush_calls;
static short flushed;
static int FlushVol(const void *name,short volume){assert(!name);assert(volume);flush_calls++;flushed=volume;return 0;}
/* Both volumes deliberately have the same parent ID and filename. */
static char files[2][32]={"clarice.conf","clarice.conf"};
static void pstr(unsigned char *p,const char *s){p[0]=strlen(s);memcpy(p+1,s,p[0]);}
static int PBHGetVInfoSync(HParamBlockRec *p){
 int i=p->volumeParam.ioVolIndex;if(fault)return -36;
 if(i==0){p->volumeParam.ioVRefNum=-1;return 0;} /* name ignored */
 if(i<1||i>2+duplicate)return nsvErr;
 pstr(p->volumeParam.ioNamePtr,i==1?"Launch":"Test 2");
 p->volumeParam.ioVRefNum=-i;return 0;
}
static int PBGetCatInfoSync(CInfoPBRec *p){
 char path[256];memcpy(path,p->hFileInfo.ioNamePtr+1,p->hFileInfo.ioNamePtr[0]);path[p->hFileInfo.ioNamePtr[0]]=0;
 if(strstr(path,"missing"))return fnfErr;
 p->hFileInfo.ioFlParID=42;return 0;
}
static int PBHRenameSync(HParamBlockRec *p){
 rename_calls++;assert(p->fileParam.ioDirID==42);
 int v=-p->fileParam.ioVRefNum-1;assert(v>=0&&v<2);
 unsigned char *old=p->fileParam.ioNamePtr,*name=(unsigned char*)p->ioParam.ioMisc;
 if(strlen(files[v])!=old[0]||memcmp(files[v],old+1,old[0]))return fnfErr;
 assert(name[0]<sizeof files[v]);memcpy(files[v],name+1,name[0]);files[v][name[0]]=0;return 0;
}
'''
main = r'''
static int rename_path(const char *path,const char *name){unsigned char p[256],n[256];pstr(p,path);pstr(n,name);return shell_rename(p,n);}
int main(void){
 assert(!rename_path("Test 2:folder:clarice.conf","clarice.conf.bak"));
 assert(!strcmp(files[0],"clarice.conf"));assert(!strcmp(files[1],"clarice.conf.bak"));
 assert(!rename_path("test 2:folder:clarice.conf.bak","clarice.conf"));
 assert(!strcmp(files[0],"clarice.conf"));assert(!strcmp(files[1],"clarice.conf"));
 assert(!rename_path("Launch:folder:clarice.conf","launch.bak"));assert(!strcmp(files[1],"clarice.conf"));
 assert(!shell_flush_native("Test 2:folder:clarice.conf"));assert(flushed==-2);
 assert(!shell_flush_native("Launch:folder:launch.bak"));assert(flushed==-1);
 int calls=rename_calls,flushes=flush_calls;
 assert(shell_flush_native("Missing:file")==nsvErr);
 assert(shell_flush_native(":relative")==bdNamErr);
 assert(rename_path("Missing:folder:clarice.conf","new")==nsvErr);
 assert(rename_path("Test 2:missing:clarice.conf","new")==fnfErr);
 assert(rename_path(":folder:clarice.conf","new")==bdNamErr);
 assert(rename_path("clarice.conf","new")==bdNamErr);
 duplicate=1;assert(rename_path("Test 2:folder:clarice.conf","new")==dupFNErr);duplicate=0;
 fault=1;assert(shell_flush_native("Test 2:folder:clarice.conf")==-36);assert(rename_path("Test 2:folder:clarice.conf","new")==-36);fault=0;
 assert(rename_calls==calls);assert(flush_calls==flushes);assert(!strcmp(files[0],"launch.bak"));assert(!strcmp(files[1],"clarice.conf"));
 puts("PASS: cross-volume rename/recovery/flush, duplicate IDs/leaves, spaces/case, missing/ambiguous/error fail closed");
}
'''
with tempfile.TemporaryDirectory() as d:
    c, exe = Path(d) / 'test.c', Path(d) / 'test'
    c.write_text(preamble + helpers + rename + main)
    subprocess.run(['cc', '-std=c99', '-fsanitize=address,undefined', str(c), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
    # The old lookup's default-volume result must fail even when rename itself succeeds.
    broken = rename.replace('e=cli_native_volume(path,&vr);', 'e=0;vr=-1;')
    assert broken != rename
    c.write_text(preamble + helpers + broken + main)
    subprocess.run(['cc', '-std=c99', str(c), '-o', str(exe)], check=True)
    assert subprocess.run([str(exe)], capture_output=True).returncode != 0
    print('PASS: regression detects a successful rename on the wrong volume')
