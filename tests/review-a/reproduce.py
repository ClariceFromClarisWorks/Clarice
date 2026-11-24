"""Execute pre-fix source fragments with focused fault/native stubs."""
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];w=r/'work/review-a';w.mkdir(parents=True,exist_ok=True)
s=subprocess.check_output(['git','show','397c2af:src/offline-install.inc'],cwd=r,text=True);cleanup=s[s.index('if(result.error){'):s.index('\n say("Installation complete.')]
rotation=s[s.index(' if(present){\n  char media'):s.index('\n if(copy&&')]
h=subprocess.check_output(['git','show','397c2af:src/offline-recovery.inc'],cwd=r,text=True);helper=h[h.index(' CInfoPBRec cat;e=info(helper'):h.index('\n RecoverySnapshot s;')]
repo=subprocess.check_output(['git','show','397c2af:src/commands/repository-recovery.inc'],cwd=r,text=True);retry=repo[repo.index(' if(!e){RecoverySnapshot s;'):repo.index('\n if(e!=fnfErr)return e;')]
code=r'''
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define fnfErr -43
#define paramErr -50
#include "src/update/transaction.h"
static int staged=1,rotated,verified;
static struct {char stage[256],backup[256];unsigned original_id,original_modified;} offline_plan={"Disk:stage"};
static int cli_native_delete(const char *p){(void)p;staged=0;return 0;}
static void errsay(const char *s){(void)s;}
static int cleanup_test(void){UpdateResult result={-128,0,0,0,1};
'''+cleanup+r'''
return 0;}
static int offline_recovery(const char *a,const char *b,unsigned c){(void)a;(void)b;(void)c;rotated++;return 0;}
typedef int RecoveryFile;
typedef struct {struct {unsigned ioDirID,ioFlMdDat;}hFileInfo;} CInfoPBRec;
static int info(const char *p,CInfoPBRec *c){(void)p;memset(c,0,sizeof *c);return 0;}
static int recovery_copy(const char*a,const char*b,const char*c,RecoveryFile*d){return 0;}
static int rotation_test(void){int present=1,copy=0,migrating=0,e;unsigned installed=10,build=10;char own[256]="Disk:Kit:",origin[256]="Disk:Clarice",target[256]="Disk:Clarice";CInfoPBRec original={0};
'''+rotation+r'''
return 0;}
static unsigned offline_helper_resource=100,offline_helper_resource_crc=123;
static int update_hash_fork(const char*p,int fork,uint32_t *crc,unsigned n){(void)p;(void)fork;(void)n;*crc=123;return 0;}
static int helper_test(void){char helper[256]="wrong-type-added-data";char helper_source[256]="source";int e;uint32_t crc;
'''+helper+r'''
return 0;}
typedef struct {struct {char root[256];}header;} RecoverySnapshot;
static int recovery_inventory(const char *p,RecoverySnapshot*s,int write){(void)p;(void)write;strcpy(s->header.root,"root");return 0;}
static int recovery_verify(const char *p,RecoverySnapshot*s){(void)p;(void)s;verified++;return 0;}
static void recovery_dispose(RecoverySnapshot*s){(void)s;}
static int retry_test(void){int e=0;char root[]="root";struct{char path[1][256];}rotation={{{0}}};
'''+retry+r'''
return -1;}
int main(void){cleanup_test();printf("P1-1 staged Core retained=%d (wanted1)\n",staged);rotation_test();printf("P1-3 unchanged install rotated=%d (wanted0)\n",rotated);printf("P1-5 bad helper accepted=%d (wanted0)\n",helper_test()==0);printf("P1-2 changed-index retry accepted=%d (wanted0)\n",retry_test()==0);return 0;}
'''
(w/'reproduce.c').write_text(code)
subprocess.run(['cc','-Wno-multichar','-I',str(r),str(w/'reproduce.c'),'-o',str(w/'reproduce')],check=True)
subprocess.run([str(w/'reproduce')],check=True)
