"""Exercise the production per-frame enumerator against hostile HFS result blocks."""
from pathlib import Path
import subprocess,tempfile
s=Path('src/commands/files.c').read_text();s=s[s.index('static int directory_nth'):s.index('typedef struct RemoveFrame')]
pre=r'''
#include <string.h>
#include <assert.h>
#include <stdio.h>
#define bdNamErr -37
static int calls,fault,length=31;
typedef struct {struct {unsigned char*ioNamePtr;short ioVRefNum,ioFDirIndex;long ioDirID;} hFileInfo;} CInfoPBRec;
static int PBGetCatInfoSync(CInfoPBRec*p){calls++;assert(p->hFileInfo.ioVRefNum==-2&&p->hFileInfo.ioDirID==123&&p->hFileInfo.ioFDirIndex==7);if(fault)return -36;unsigned char*n=p->hFileInfo.ioNamePtr;n[0]=length;memset(n+1,'x',length);p->hFileInfo.ioDirID=456;p->hFileInfo.ioVRefNum=0;return 0;}
'''
post=r'''
int main(void){char leaf[32];CInfoPBRec p;memset(&p,0xee,sizeof p);for(int i=0;i<3;i++){assert(!directory_nth(-2,123,7,leaf,&p));assert(strlen(leaf)==31&&p.hFileInfo.ioDirID==456);}
length=32;assert(directory_nth(-2,123,7,leaf,&p)==bdNamErr);fault=1;assert(directory_nth(-2,123,7,leaf,&p)==-36);fault=0;length=1;assert(!directory_nth(-2,123,7,leaf,&p)&&!strcmp(leaf,"x"));assert(calls==6);puts("PASS explicit volume/parent restored after overwritten results, leaf bounds, catalog failure and retry");}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(pre+s+post);subprocess.run(['cc','-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
