"""Execute production preflight/no-op branches; verify publisher/guest framing."""
from pathlib import Path
import subprocess,tempfile,sys,hashlib,binascii
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'scripts'))
import repository
s=(r/'src/commands/repository-client.inc').read_text()
branch=s[s.index('if(r->core){core=i;'):s.index('}else{\n   char path',s.index('if(r->core){core=i;'))]+'}'
noop=s[s.index(' if(!changes){char native'):s.index('\n if(changes&&!yes)')]
code=r'''
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define paramErr -50
#define fnfErr -43
#define PATH_CAP 256
#define REPO_MARKER "marker"
typedef struct {int core;char name[32];unsigned version,size,crc;} RepoRecord;
typedef struct RepoItem {RepoRecord wanted,installed;int known,needed;struct RepoItem *next;}RepoItem;
typedef int CInfoPBRec;
static int marker,reads,writes,catalog_error;
static int checked(const char*p,char*n,char*c,int v){strcpy(n,p);return 0;}
static int info(const char*p,CInfoPBRec*c){reads++;return marker?marker<0?marker:0:fnfErr;}
static void say(const char*p){}static void errsay(const char*p){}
static int repo_catalog(void*p,RepoRecord*r){writes++;return catalog_error;}
static int preflight(unsigned current,unsigned version,int repository_migrate){int e=0;RepoItem item={0},*i=&item,*core=0;RepoRecord*r=&i->wanted;r->core=1;r->version=version;
'''+branch+r'''
return i->needed?1:0;done:return e;}
static int noop(RepoItem*items,int changes,unsigned current){int e=0;RepoItem*core=items;
'''+noop+r'''
return 99;done:return e;}
int main(void){
assert(preflight(20,10,1)==paramErr);assert(preflight(20,20,1)==1);assert(preflight(20,21,1)==1);assert(preflight(20,10,0)==0);
RepoItem tool={{0,"ls",20,10,30},{0,"ls",20,10,30},1,0,0},core={{1,"Clarice",20,0,0},{1,"Clarice",20,0,0},1,0,&tool};
assert(!noop(&core,0,20)&&reads==1&&!writes);tool.known=0;assert(!noop(&core,0,20)&&writes==1);tool.known=1;core.known=0;assert(!noop(&core,0,20)&&writes==2);core.known=1;
marker=1;assert(noop(&core,0,20)==99&&writes==2);marker=-36;assert(noop(&core,0,20)==-36);marker=0;assert(noop(&core,1,20)==99);tool.known=0;catalog_error=-34;assert(noop(&core,0,20)==-34);
puts("PASS older migration rejected; no-op skips recovery/markers; adoption, pending retry and errors preserved");}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(code)
 subprocess.run(['cc','-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
 data=b'core';leaf=hashlib.sha256(data).hexdigest()[:24]+'.bin';(p/leaf).write_bytes(data)
 good=f'CLARICE-REPOSITORY 1\ncore\tClarice\t1\t4\t{binascii.crc32(data):08x}\t{leaf}\n'.encode()
 for bad in [good[:-1],good.replace(b'\n',b'\r\n'),good.replace(b'\n',b'\r'),good+b'\0',good.replace(b'\n',b'\x0b'),good.replace(b'Clarice\t1',b'x'*256+b'\t1')]:
  (p/'index.txt').write_bytes(bad)
  try:repository.verified_repository(p)
  except ValueError:pass
  else:raise AssertionError('guest-incompatible framing accepted')
 (p/'index.txt').write_bytes(good);repository.verified_repository(p)
 print('PASS repository LF/final newline/line-capacity validation')
