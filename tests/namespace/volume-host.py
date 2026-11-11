"""Compile the actual volume/catalog helpers against input-only HFS PB mocks."""
from pathlib import Path
import subprocess, tempfile

def function(source, name):
    start=source.index('static int '+name+'(')
    brace=source.index('{',start); depth=1; end=brace+1
    while depth:
        depth += (source[end]=='{')-(source[end]=='}'); end+=1
    return source[start:end]
ns=Path('src/namespace.inc').read_text(); shell=Path('src/shell.c').read_text()
helpers='\n'.join(function(ns,n) for n in ['same_name','volume_lookup','cli_native_volume'])
info=function(shell,'info')
preamble=r'''
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
enum {nsvErr=-35,dupFNErr=-48,bdNamErr=-37,fnfErr=-43};
typedef struct {unsigned char *ioNamePtr;short ioVolIndex,ioVRefNum;} Volume;
typedef struct {Volume volumeParam;} HParamBlockRec;
typedef struct {unsigned char *ioNamePtr;short ioVRefNum,ioFDirIndex;long ioDirID;} Catalog;
typedef struct {Catalog hFileInfo;} CInfoPBRec;
static int duplicate, fault, catcalls;
static void pstr(unsigned char *p,const char *s){p[0]=strlen(s);memcpy(p+1,s,p[0]);}
static int PBHGetVInfoSync(HParamBlockRec *p){
 int i=p->volumeParam.ioVolIndex;if(fault)return -36;
 if(i<1||i>2+duplicate)return nsvErr;
 pstr(p->volumeParam.ioNamePtr,i==1?"Boot":"Test 2");
 p->volumeParam.ioVRefNum=-i;return 0;
}
static int PBGetCatInfoSync(CInfoPBRec *p){
 /* Full-name lookup selects the right catalog but does NOT rewrite vRefNum.
    Indexed lookup uses the input reference (zero means boot volume). */
 catcalls++;
 if(p->hFileInfo.ioFDirIndex){return p->hFileInfo.ioVRefNum==-2?0:fnfErr;}
 p->hFileInfo.ioDirID=2;return 0;
}
'''
main=r'''
int main(void){CInfoPBRec pb;short ref=77;
 assert(!info("Test 2:",&pb));assert(pb.hFileInfo.ioVRefNum==-2);
 pb.hFileInfo.ioFDirIndex=1;assert(!PBGetCatInfoSync(&pb));
 assert(!info("Boot:folder",&pb));assert(pb.hFileInfo.ioVRefNum==-1);
 assert(!info("test 2:sub:file",&pb));assert(pb.hFileInfo.ioVRefNum==-2);
 int calls=catcalls;
 assert(info("Missing:file",&pb)==nsvErr);assert(catcalls==calls);
 assert(info(":relative",&pb)==bdNamErr);assert(info("file",&pb)==bdNamErr);
 duplicate=1;assert(info("Test 2:file",&pb)==dupFNErr);assert(catcalls==calls);
 duplicate=0;fault=1;assert(info("Boot:file",&pb)==-36);assert(catcalls==calls);
 assert(cli_native_volume("Boot:file",&ref)==-36);assert(ref==77);
 puts("PASS: explicit refs, case/spaces, indexed lookup, absent/ambiguous/invalid/error fail closed");
}
'''
with tempfile.TemporaryDirectory() as d:
    c=Path(d)/'test.c'; exe=Path(d)/'test'
    c.write_text(preamble+helpers+info+main)
    subprocess.run(['cc','-std=c99','-fsanitize=address,undefined',str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
    # Mutation check: the previous zero-reference implementation must fail.
    c.write_text(preamble+helpers+info.replace('ioVRefNum=vol','ioVRefNum=0')+main)
    subprocess.run(['cc','-std=c99',str(c),'-o',str(exe)],check=True)
    result=subprocess.run([str(exe)],capture_output=True)
    assert result.returncode != 0
    print('PASS: regression rejects previous zero-reference behavior')
