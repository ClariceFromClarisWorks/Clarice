"""Test the production helper's interrupted-rename decisions; native HFS is Snow-tested separately."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[2];s=(root/'src/update/helper.c').read_text();body=s[s.index(' CInfoPBRec target,backup;'):s.index(' /* Remove only')]
code=r'''
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "src/update/transaction.h"
#define fnfErr -43
#define paramErr -50
#define permErr -54
typedef struct {struct {uint32_t ioDirID,ioFlMdDat;} hFileInfo;} CInfoPBRec;
static struct {char *target,*backup;uint32_t original_id,original_modified;} plan={"target","backup",42,123};
static int target_id,backup_id,commits,restores,flush_error,busy;
static int update_info(const char *p,CInfoPBRec *c){int id=!strcmp(p,"target")?target_id:backup_id;if(!id)return fnfErr;c->hFileInfo.ioDirID=id;c->hFileInfo.ioFlMdDat=123;return 0;}
static int update_verify_candidate(const char *p,void *plan){(void)p;(void)plan;return target_id==43?0:paramErr;}
static int closed(void *c,int n){(void)c;(void)n;return busy?-47:0;}
static int update_rename(const char *a,const char *b){(void)a;(void)b;assert(!target_id);target_id=backup_id;backup_id=0;restores++;return 0;}
static int update_flush(const char *p){(void)p;return flush_error;}
static int probe(void *c,int n){(void)c;(void)n;return target_id==42?1:permErr;}
static int verify(void *c,int n){(void)c;(void)n;return 0;}
static int move(void *c,int a,int b){(void)c;(void)a;(void)b;return 0;}
static int flush(void *c){(void)c;return 0;}
static void explain(const char *s){(void)s;}
#define trace(s) ((void)0)
static UpdateResult fake_commit(const UpdateOps *o){(void)o;commits++;UpdateResult r={0};return r;}
#define update_commit fake_commit
static int recover(void){int e=0;
'''+body+r'''
return 0;}
static void reset(int target,int backup){target_id=target;backup_id=backup;commits=restores=flush_error=busy=0;}
int main(void){
 reset(42,0);assert(!recover()&&commits==1&&!restores);
 reset(43,42);assert(!recover()&&!commits&&!restores);
 reset(0,42);assert(!recover()&&commits==1&&restores==1&&target_id==42);
 reset(44,42);assert(recover()&&!commits&&!restores&&target_id==44);
 reset(0,44);assert(recover()&&!commits&&!restores&&!target_id);
 reset(0,42);busy=1;assert(recover()&&!commits&&!restores);
 reset(0,42);flush_error=-36;assert(recover()&&!commits&&target_id==42);
 puts("PASS production helper recovery decisions: original, already installed, interrupted rename, unrelated target/backup, busy and failed flush");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(code);subprocess.run(['cc','-fsanitize=address,undefined','-I',str(root),str(p/'test.c'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
