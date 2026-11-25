/* Production recovery code with an injected native HFS model. These are host
   ownership/failure tests, not physical durability or power-loss acceptance. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#define CLARICE_UPDATE_NATIVE_FILES_H
#define CLARICE_UPDATE_OPEN_GUARD_H
#define CLARICE_UPDATE_STAMP_H
#define RECOVERY_COMMAND 1
#define fnfErr -43
#define dirNFErr -120
#define paramErr -50
#define bdNamErr -37
#define ioErr -36
#define eofErr -39
#define memFullErr -108
#define permErr -54
#define fBsyErr -47
#define dupFNErr -48
#define fsFromStart 1
typedef char *Ptr;
typedef struct {uint32_t fdType,fdCreator;unsigned char rest[8];} Finder;
typedef struct {struct {unsigned char *ioNamePtr;short ioVRefNum,ioFDirIndex;int ioFlAttrib;uint32_t ioDirID,ioFlParID,ioFlLgLen,ioFlRLgLen,ioFlCrDat,ioFlMdDat;Finder ioFlFndrInfo;unsigned char ioFlXFndrInfo[16];} hFileInfo;struct {unsigned char *ioNamePtr;short ioVRefNum,ioFDirIndex;long ioDrDirID,ioDrParID;} dirInfo;} CInfoPBRec;
static int boundaries,fail_at,crash_at,allocs,fail_alloc,identity=100,fault_code=ioErr,cancel_poll;
#define RECOVERY_POLL() (cancel_poll ? -128 : 0)
static int native_scratch;
static jmp_buf reset_point;
static void *live[128];
static int boundary_at(const char *where){boundaries++;if(boundaries==fail_at&&getenv("TRACE_FAULT"))fprintf(stderr,"failure %d at %s\n",boundaries,where);if(boundaries==crash_at)longjmp(reset_point,1);return boundaries==fail_at?fault_code:0;}
#define boundary() boundary_at(__func__)
static Ptr NewPtr(unsigned long n){if(fail_alloc)return 0;void *p=malloc(n);if(p){allocs++;int i=0;while(i<128&&live[i])i++;assert(i<128);live[i]=p;}return p;}
static void DisposePtr(Ptr p){assert(allocs>0);allocs--;int i=0;while(i<128&&live[i]!=p)i++;assert(i<128);live[i]=0;free(p);}
#define RECOVERY_ALLOC(n) NewPtr(n)
#define RECOVERY_FREE(p) DisposePtr((Ptr)(p))
struct File {char path[256];CInfoPBRec info;unsigned char fork[2][8192];int open;};
static struct File fs[100],baseline[100];
static struct Ref {int used,file,fork,pos,write;} refs[32];
static int find(const char *p){for(int i=0;i<100;i++)if(fs[i].path[0]&&!strcmp(p,fs[i].path))return i;return -1;}
static int update_volume(const char *p,short *v){if(strncmp(p,"Disk:",5))return paramErr;*v=1;return 0;}
static void update_pstr(unsigned char *p,const char *s){p[0]=strlen(s);memcpy(p+1,s,p[0]);}
static int update_info(const char *p,CInfoPBRec *c){int e=boundary();if(e)return e;int i=find(p);if(i<0)return fnfErr;*c=fs[i].info;return 0;}
static int cli_native_create(const char *p,unsigned long type,unsigned long creator){int e=boundary();if(e)return e;if(find(p)>=0)return dupFNErr;for(int i=0;i<100;i++)if(!fs[i].path[0]){memset(&fs[i],0,sizeof fs[i]);strcpy(fs[i].path,p);fs[i].info.hFileInfo.ioDirID=++identity;fs[i].info.hFileInfo.ioFlFndrInfo.fdType=type;fs[i].info.hFileInfo.ioFlFndrInfo.fdCreator=creator;fs[i].info.hFileInfo.ioFlCrDat=fs[i].info.hFileInfo.ioFlMdDat=77;return 0;}return memFullErr;}
static int cli_native_mkdir(const char *p){int e=cli_native_create(p,0,0);if(!e){fs[find(p)].info.hFileInfo.ioFlAttrib=16;fs[find(p)].info.dirInfo.ioDrDirID=fs[find(p)].info.hFileInfo.ioDirID;}return e;}
static int cli_native_lock(const char *p,int locked){int e=boundary();if(e)return e;int i=find(p);if(i<0)return fnfErr;fs[i].info.hFileInfo.ioFlAttrib=(fs[i].info.hFileInfo.ioFlAttrib&~1)|!!locked;return 0;}
static int cli_native_delete(const char *p){int e=boundary();if(e)return e;int i=find(p);if(i<0)return fnfErr;if(fs[i].open)return fBsyErr;if(fs[i].info.hFileInfo.ioFlAttrib&1)return permErr;size_t n=strlen(p);for(int j=0;j<100;j++)if(!strncmp(fs[j].path,p,n)&&fs[j].path[n]==':')return fBsyErr;fs[i].path[0]=0;return 0;}
static int cli_native_set(const char *p,CInfoPBRec *c){int e=boundary();if(e)return e;int i=find(p);if(i<0)return fnfErr;uint32_t id=fs[i].info.hFileInfo.ioDirID,d=fs[i].info.hFileInfo.ioFlLgLen,r=fs[i].info.hFileInfo.ioFlRLgLen;int attr=fs[i].info.hFileInfo.ioFlAttrib;fs[i].info=*c;fs[i].info.hFileInfo.ioFlAttrib=attr;fs[i].info.hFileInfo.ioDirID=id;fs[i].info.hFileInfo.ioFlLgLen=d;fs[i].info.hFileInfo.ioFlRLgLen=r;return 0;}
static int cli_native_rename(const char *a,const char *b){const char *x=strrchr(a,':'),*y=strrchr(b,':');if(!x||!y||x-a!=y-b||memcmp(a,b,x-a))return paramErr;int e=boundary();if(e)return e;int i=find(a);if(i<0)return fnfErr;if(find(b)>=0)return dupFNErr;size_t n=strlen(a);for(int j=0;j<100;j++)if(!strcmp(fs[j].path,a)||(!strncmp(fs[j].path,a,n)&&fs[j].path[n]==':')){char path[256];snprintf(path,sizeof path,"%s%s",b,fs[j].path+n);strcpy(fs[j].path,path);}return 0;}
static int update_rename(const char *a,const char *b){return cli_native_rename(a,b);}
static int update_flush(const char *p){(void)p;return boundary();}
static int update_open_guard(const char *p,int *busy){int i=find(p);if(i<0)return fnfErr;*busy=fs[i].open;return 0;}
static int update_open(const char *p,int fork,int write,short *r){int e=boundary();if(e)return e;int i=find(p);if(i<0){const char *leaf=strrchr(p,':');if(leaf&&!strcmp(leaf+1,"Inventory")){char parent[256];memcpy(parent,p,leaf-p);parent[leaf-p]=0;if(find(parent)<0)return dirNFErr;}return fnfErr;}for(int j=0;j<32;j++)if(!refs[j].used){refs[j]=(struct Ref){1,i,fork,0,write};fs[i].open++;*r=j;return 0;}return fBsyErr;}
static int GetEOF(short r,long *size){struct Ref *f=&refs[r];assert(f->used);*size=f->fork?fs[f->file].info.hFileInfo.ioFlRLgLen:fs[f->file].info.hFileInfo.ioFlLgLen;return boundary();}
static int FSRead(short r,long *n,void *p){int e=boundary();if(e)return e;struct Ref *f=&refs[r];assert(f->used);long size=f->fork?fs[f->file].info.hFileInfo.ioFlRLgLen:fs[f->file].info.hFileInfo.ioFlLgLen;long want=*n;if(*n>size-f->pos)*n=size-f->pos;memcpy(p,fs[f->file].fork[f->fork]+f->pos,*n);f->pos+=*n;return *n==want?0:eofErr;}
static int FSWrite(short r,long *n,const void *p){int e=boundary();if(e)return e;struct Ref *f=&refs[r];assert(f->used&&f->write);if(f->pos+*n>8192)return -34;memcpy(fs[f->file].fork[f->fork]+f->pos,p,*n);f->pos+=*n;if(f->fork)fs[f->file].info.hFileInfo.ioFlRLgLen=f->pos;else fs[f->file].info.hFileInfo.ioFlLgLen=f->pos;return 0;}
static int FSClose(short r){struct Ref *f=&refs[r];assert(f->used);if(native_scratch&&f->write&&f->fork&&fs[f->file].info.hFileInfo.ioFlRLgLen>=256)memset(fs[f->file].fork[1]+48,0x5a,80);fs[f->file].open--;f->used=0;return boundary();}
static int update_stamp(const char *path,uint32_t *build,unsigned *system){if(find(path)<0)return fnfErr;*build=1947;*system=0x600;return 0;}
#include "../../src/update/recovery-restore.h"
static void file(const char *p,const char *text){assert(!cli_native_create(p,'TEXT','CLRC'));int i=find(p);memcpy(fs[i].fork[0],text,strlen(text));fs[i].info.hFileInfo.ioFlLgLen=strlen(text);memcpy(fs[i].fork[1],"resource!",9);fs[i].info.hFileInfo.ioFlRLgLen=9;}
static void make_snapshot(const char *dir){
 assert(!cli_native_mkdir(dir));RecoverySnapshot s={0};strcpy(s.header.root,"Disk:System:Clarice Data:");strcpy(s.header.origin,"Disk:Clarice");s.header.build=1947;s.header.origin_id=fs[find("Disk:Clarice")].info.hFileInfo.ioDirID;s.header.origin_modified=77;s.header.count=5;if(strstr(dir,"Preparing"))s.header.retired_id=fs[find("Disk:System:Clarice Data:etc:clarice:Recovery Previous")].info.hFileInfo.ioDirID;s.files=(RecoveryFile*)NewPtr(5*sizeof *s.files);memset(s.files,0,5*sizeof *s.files);
 const char *names[]={"@core","@helper","@catalog","ls","nano-profile"},*sources[]={"Disk:Clarice","Disk:Helper","Disk:System:Clarice Data:etc:clarice:installed.catalog","Disk:System:Clarice Data:bin:ls","Disk:System:Clarice Data:bin:nano-profile"};
 for(int i=0;i<5;i++){char dest[256];assert(!recovery_member(dir,names[i],dest));assert(!recovery_copy(sources[i],dest,names[i],&s.files[i]));s.files[i].checksum_policy=0;assert(!recovery_fork(dest,0,1,s.files[i].resource,&s.files[i].resource_crc,recovery_normalized(&s.files[i])));}
 assert(!recovery_inventory(dir,&s,1));assert(!recovery_verify(dir,&s));recovery_dispose(&s);
}
static int prepare_snapshot(RecoveryRotation *r){
 RecoverySnapshot s={0};strcpy(s.header.root,"Disk:System:Clarice Data:");strcpy(s.header.origin,"Disk:Clarice");s.header.build=1947;s.header.origin_id=fs[find("Disk:Clarice")].info.hFileInfo.ioDirID;s.header.origin_modified=77;s.header.count=5;
 s.files=(RecoveryFile*)NewPtr(5*sizeof *s.files);if(!s.files)return memFullErr;memset(s.files,0,5*sizeof *s.files);const char *names[]={"@core","@helper","@catalog","ls","nano-profile"};for(int i=0;i<5;i++)strcpy(s.files[i].name,names[i]);
 int e=recovery_prepare(r,&s,"Disk:Helper",0);recovery_dispose(&s);return e;
}
static void reset(void){memcpy(fs,baseline,sizeof fs);memset(refs,0,sizeof refs);boundaries=fail_at=crash_at=fail_alloc=cancel_poll=0;fault_code=ioErr;assert(!allocs);}
static void process_reset(void){for(int i=0;i<128;i++)if(live[i]){free(live[i]);live[i]=0;}allocs=0;memset(refs,0,sizeof refs);for(int i=0;i<100;i++)fs[i].open=0;crash_at=fail_at=0;}
static void closed_all(void){for(int i=0;i<32;i++)assert(!refs[i].used);assert(!allocs);
 const char *kept[]={"Disk:Notes","Disk:System:Clarice Data:etc:clarice:clarice.conf","Disk:System:Clarice Data:bin:.clx-ls.old"};for(int n=0;n<3;n++){int i=find(kept[n]);assert(i>=0);assert(!strcmp((char*)fs[i].fork[0],"user-owned"));assert(!memcmp(fs[i].fork[1],"resource!",9));}}

static int validate_tool(const char *p){return find(p)<0?fnfErr:0;}
int main(void){
 {const char *names[]={"core","catalog","inventory"};for(int i=0;i<3;i++){RecoveryFile f={0};strcpy(f.name,names[i]);RecoverySnapshot snap={0};snap.header.count=1;snap.files=&f;RecoveryRotation rotation={0};int before=boundaries;assert(recovery_prepare(&rotation,&snap,"",0)==paramErr&&boundaries==before);unsigned char encoded[RECOVERY_ENTRY];assert(!recovery_file_encode(&f,encoded));}}
 puts("PASS reserved HFS snapshot names rejected before I/O; legacy codecs unchanged");
 file("Disk:Notes","user-owned");file("Disk:System:Clarice Data:etc:clarice:clarice.conf","user-owned");file("Disk:System:Clarice Data:bin:.clx-ls.old","user-owned");
 file("Disk:Clarice","old core");file("Disk:Helper","helper");file("Disk:System:Clarice Data:etc:clarice:installed.catalog","CLARICE-INSTALLED 1\n");file("Disk:System:Clarice Data:bin:ls","old tool");
 /* Catalogued optional diagnostics are opaque backup members, not executed.
    Exercise multiple bounded copy chunks in both forks. */
 file("Disk:System:Clarice Data:bin:nano-profile","archived diagnostic");
 int diagnostic=find("Disk:System:Clarice Data:bin:nano-profile");
 for(int k=0;k<6000;k++)fs[diagnostic].fork[0][k]=(unsigned char)(k*19);
 for(int k=0;k<7000;k++)fs[diagnostic].fork[1][k]=(unsigned char)(k*23);
 fs[diagnostic].info.hFileInfo.ioFlLgLen=6000;fs[diagnostic].info.hFileInfo.ioFlRLgLen=7000;
 for(int i=0;i<3;i++){
  int n=i==0?find("Disk:Clarice"):i==1?find("Disk:Helper"):diagnostic;
  unsigned char *r=fs[n].fork[1];uint32_t length=i==2?7000:540;
  fs[n].info.hFileInfo.ioFlRLgLen=length;
  recovery_put32(r,256);recovery_put32(r+4,length-28);recovery_put32(r+8,length-284);recovery_put32(r+12,28);
 }
 RecoveryRotation rotation={ {"Disk:System:Clarice Data:etc:clarice:Recovery Previous","Disk:System:Clarice Data:etc:clarice:Recovery Preparing","Disk:System:Clarice Data:etc:clarice:Recovery Retiring"} };
 make_snapshot(rotation.path[0]);
 {RecoverySnapshot legacy;assert(!recovery_inventory(rotation.path[0],&legacy,0));assert(!legacy.header.format);assert(!recovery_verify(rotation.path[0],&legacy));
  char inv[256],copy[256],launch[256];assert(!recovery_join(rotation.path[0],RECOVERY_MANIFEST,inv));int ix=find(inv);unsigned char unchanged[8192];memcpy(unchanged,fs[ix].fork[0],8192);
  assert(!recovery_member(rotation.path[0],"nano-profile",copy));fs[find(copy)].fork[1][48]^=1;assert(recovery_verify(rotation.path[0],&legacy)==paramErr);fs[find(copy)].fork[1][48]^=1;
  fs[diagnostic].fork[0][0]^=1;native_scratch=1;assert(!recovery_restore(rotation.path[0],launch));native_scratch=0;
  assert(!memcmp(unchanged,fs[ix].fork[0],8192));assert(!recovery_verify(rotation.path[0],&legacy));recovery_dispose(&legacy);closed_all();
  puts("PASS unchanged CLRSNAP1 inventory: exact legacy CRC, corruption rejection, native-copy rollback");
 }
 make_snapshot(rotation.path[1]);
 /* A successful call must not authorize a later call using the same context. */
 {char path[256];assert(!recovery_member(rotation.path[0],"@core",path));assert(!recovery_rotation_check(&rotation,0));fs[find(path)].fork[0][0]^=1;
  assert(!recovery_rename(rotation.path[1],rotation.path[2]));assert(recovery_rotate(&rotation)==paramErr);assert(find(rotation.path[2])>=0);
  fs[find(path)].fork[0][0]^=1;assert(!recovery_rename(rotation.path[2],rotation.path[1]));}
 memcpy(baseline,fs,sizeof fs);boundaries=0;assert(!recovery_rotate(&rotation));int total=boundaries;closed_all();
 for(int fault=1;fault<=total;fault++){
  reset();fail_at=fault;(void)recovery_rotate(&rotation);fail_at=0;closed_all();
  int valid=0;for(int n=0;n<3;n++){RecoverySnapshot s;if(!recovery_inventory(rotation.path[n],&s,0)){valid+=!recovery_verify(rotation.path[n],&s);recovery_dispose(&s);}}
  assert(valid);closed_all();int retry=recovery_rotate(&rotation);if(retry){fprintf(stderr,"rotation retry failed boundary=%d error=%d\n",fault,retry);return 1;}closed_all();
 }
 /* Interrupted CLRSNAP1 inventory writes resume with old checksum semantics. */
 for(int bytes=8;bytes<=RECOVERY_HEADER+RECOVERY_ENTRY;bytes+=RECOVERY_HEADER-8){
  reset();char inv[256];assert(!recovery_join(rotation.path[1],RECOVERY_MANIFEST,inv));fs[find(inv)].info.hFileInfo.ioFlLgLen=bytes;
  assert(!prepare_snapshot(&rotation));RecoverySnapshot legacy;assert(!recovery_inventory(rotation.path[0],&legacy,0));assert(!legacy.header.format);assert(!recovery_verify(rotation.path[0],&legacy));recovery_dispose(&legacy);closed_all();
 }
 puts("PASS legacy interrupted inventory resumes without checksum reinterpretation");
 /* A new installer replaces only a duplicate helper backed by verified old
    recovery. Faults leave that generation intact and retry resumes. */
 reset();{char hp[256];assert(!recovery_member(rotation.path[1],"@helper",hp));fs[find(hp)].info.hFileInfo.ioFlCrDat=fs[find(hp)].info.hFileInfo.ioFlMdDat=91;}{char p[256];assert(!recovery_join(rotation.path[1],RECOVERY_MANIFEST,p));assert(!cli_native_delete(p));}fs[find("Disk:Helper")].fork[0][0]^=1;boundaries=0;assert(!prepare_snapshot(&rotation));int helper_steps=boundaries;closed_all();
 for(int crash=0;crash<2;crash++)for(int fault=1;fault<=helper_steps;fault++){
  reset();{char hp[256];assert(!recovery_member(rotation.path[1],"@helper",hp));fs[find(hp)].info.hFileInfo.ioFlCrDat=fs[find(hp)].info.hFileInfo.ioFlMdDat=91;}{char p[256];assert(!recovery_join(rotation.path[1],RECOVERY_MANIFEST,p));assert(!cli_native_delete(p));}fs[find("Disk:Helper")].fork[0][0]^=1;boundaries=0;
  if(crash){crash_at=fault;if(!setjmp(reset_point))(void)prepare_snapshot(&rotation);process_reset();}else{fail_at=fault;(void)prepare_snapshot(&rotation);fail_at=0;closed_all();}assert(!prepare_snapshot(&rotation));closed_all();
 }
 reset();{char helper[256],inv[256];assert(!recovery_member(rotation.path[1],"@helper",helper));fs[find(helper)].fork[0][0]^=2;assert(!recovery_join(rotation.path[1],RECOVERY_MANIFEST,inv));assert(!cli_native_delete(inv));fs[find("Disk:Helper")].fork[0][0]^=1;assert(prepare_snapshot(&rotation)==paramErr);assert(fs[find(helper)].fork[0][0]==((unsigned char)'h'^2));closed_all();}
 printf("PASS previous-helper retry: %d error/interruption boundaries, unknown helper retained\n",helper_steps);
 /* Prepare from a live installation; fail/reset at each native boundary. */
 reset();assert(!recovery_prune(rotation.path[1],fs[find(rotation.path[1])].info.hFileInfo.ioDirID));memcpy(baseline,fs,sizeof fs);
 reset();fail_alloc=1;assert(prepare_snapshot(&rotation)==memFullErr);fail_alloc=0;closed_all();assert(!prepare_snapshot(&rotation));closed_all();reset();
 boundaries=0;assert(!prepare_snapshot(&rotation));int prepare_steps=boundaries;closed_all();
 for(int crash=0;crash<2;crash++)for(int fault=1;fault<=prepare_steps;fault++){
  reset();if(crash){crash_at=fault;if(!setjmp(reset_point))(void)prepare_snapshot(&rotation);process_reset();}else{fail_at=fault;(void)prepare_snapshot(&rotation);fail_at=0;closed_all();}
  int valid=0;for(int n=0;n<3;n++){RecoverySnapshot s;if(!recovery_inventory(rotation.path[n],&s,0)){valid+=!recovery_verify(rotation.path[n],&s);recovery_dispose(&s);}}assert(valid);
  int retry=prepare_snapshot(&rotation);if(retry){fprintf(stderr,"prepare retry blocked crash=%d boundary=%d error=%d\n",crash,fault,retry);return 1;}closed_all();
 }
 reset();assert(!prepare_snapshot(&rotation));RecoverySnapshot prefix;
 assert(!recovery_inventory(rotation.path[0],&prefix,0));char copy[256];
 assert(!recovery_member(rotation.path[0],"@core",copy));fs[find(copy)].fork[1][48]^=17;
 assert(!recovery_resume_copy("Disk:Clarice",copy,&prefix.files[0]));
 fs[find(copy)].fork[1][128]^=17;assert(recovery_resume_copy("Disk:Clarice",copy,&prefix.files[0])==paramErr);

 assert(!recovery_member(rotation.path[0],"nano-profile",copy));
 RecoveryFile *nf=&prefix.files[4];assert(nf->checksum_policy==1);
 fs[find(copy)].fork[1][48]^=17;assert(!recovery_check_file(copy,nf,1));
 assert(!recovery_resume_copy("Disk:System:Clarice Data:bin:nano-profile",copy,nf));
 fs[find(copy)].fork[1][300]^=1;assert(recovery_check_file(copy,nf,1)==paramErr);fs[find(copy)].fork[1][300]^=1;
 fs[find(copy)].fork[1][6990]^=1;assert(recovery_check_file(copy,nf,1)==paramErr);fs[find(copy)].fork[1][6990]^=1;
 fs[find(copy)].fork[1][0]^=1;assert(recovery_check_file(copy,nf,1)==paramErr);fs[find(copy)].fork[1][0]^=1;
 fs[find(copy)].fork[0][17]^=1;assert(recovery_check_file(copy,nf,1)==paramErr);fs[find(copy)].fork[0][17]^=1;

 {RecoveryFile raw;file("Disk:Opaque","opaque");int n=find("Disk:Opaque");fs[n].info.hFileInfo.ioFlRLgLen=300;memset(fs[n].fork[1],17,300);assert(!recovery_copy("Disk:Opaque","Disk:Opaque Copy","opaque",&raw));assert(raw.checksum_policy==2);fs[find("Disk:Opaque Copy")].fork[1][48]^=1;assert(recovery_check_file("Disk:Opaque Copy",&raw,1)==paramErr);}
 puts("PASS explicit resource policy: mutable scratch accepted; payload/map/header/data corruption rejected");
 recovery_dispose(&prefix);closed_all();
 printf("PASS snapshot preparation: %d returned-error and %d interruption boundaries\n",prepare_steps,prepare_steps);
 /* First-ever snapshot has no previous generation: repeat the same failure
    coverage rather than assuming a validated older snapshot exists. */
 reset();assert(!recovery_prune(rotation.path[0],fs[find(rotation.path[0])].info.hFileInfo.ioDirID));memcpy(baseline,fs,sizeof fs);
 boundaries=0;assert(!prepare_snapshot(&rotation));int first_steps=boundaries;closed_all();
 for(int crash=0;crash<2;crash++)for(int fault=1;fault<=first_steps;fault++){
  reset();if(crash){crash_at=fault;if(!setjmp(reset_point))(void)prepare_snapshot(&rotation);process_reset();}else{fail_at=fault;(void)prepare_snapshot(&rotation);fail_at=0;closed_all();}
  assert(!memcmp(fs[find("Disk:Clarice")].fork[0],"old core",8));
  assert(!prepare_snapshot(&rotation));closed_all();
 }
 printf("PASS first snapshot: %d error and interruption boundaries\n",first_steps);

 reset();fs[find("Disk:System:Clarice Data:bin:ls")].info.hFileInfo.ioFlAttrib|=1;memcpy(baseline,fs,sizeof fs);
 boundaries=0;assert(!prepare_snapshot(&rotation));int locked_steps=boundaries;closed_all();
 for(int crash=0;crash<2;crash++)for(int fault=1;fault<=locked_steps;fault++){
  reset();if(crash){crash_at=fault;if(!setjmp(reset_point))(void)prepare_snapshot(&rotation);process_reset();}else{fail_at=fault;(void)prepare_snapshot(&rotation);fail_at=0;closed_all();}
  assert(!prepare_snapshot(&rotation));assert(fs[find("Disk:System:Clarice Data:bin:ls")].info.hFileInfo.ioFlAttrib&1);
  assert(fs[find("Disk:System:Clarice Data:etc:clarice:Recovery Previous:ls")].info.hFileInfo.ioFlAttrib&1);closed_all();
 }
 reset();assert(!prepare_snapshot(&rotation));assert(!cli_native_lock("Disk:System:Clarice Data:bin:ls",0));
 fs[find("Disk:System:Clarice Data:bin:ls")].fork[0][0]^=1;
 struct File *before_locked_restore=malloc(sizeof baseline);assert(before_locked_restore);memcpy(before_locked_restore,baseline,sizeof baseline);memcpy(baseline,fs,sizeof fs);
 char lock_launch[256];boundaries=0;assert(!recovery_restore(rotation.path[0],lock_launch));int locked_restore_steps=boundaries;
 for(int crash=0;crash<2;crash++)for(int fault=1;fault<=locked_restore_steps;fault++){
  reset();if(crash){crash_at=fault;if(!setjmp(reset_point))(void)recovery_restore(rotation.path[0],lock_launch);process_reset();}else{fail_at=fault;(void)recovery_restore(rotation.path[0],lock_launch);fail_at=0;closed_all();}
  assert(!recovery_restore(rotation.path[0],lock_launch));assert(fs[find("Disk:System:Clarice Data:bin:ls")].info.hFileInfo.ioFlAttrib&1);closed_all();
 }
 memcpy(baseline,before_locked_restore,sizeof baseline);free(before_locked_restore);
 printf("PASS locked restore %d errors + resets, lock restored after retry\n",locked_restore_steps);assert(fs[find("Disk:System:Clarice Data:bin:ls")].info.hFileInfo.ioFlAttrib&1);closed_all();
 assert(!prepare_snapshot(&rotation));closed_all(); /* prune the retired locked snapshot */
 printf("PASS locked source preserved, snapshot/restore lock and rotation: %d errors + resets\n",locked_steps);
 reset();fs[find("Disk:System:Clarice Data:bin:ls")].info.hFileInfo.ioFlAttrib&=~1;memcpy(baseline,fs,sizeof fs);
 reset();assert(!prepare_snapshot(&rotation));memcpy(baseline,fs,sizeof fs);
 reset();char current[256];strcpy(current,rotation.path[0]);int core=find("Disk:Clarice"),tool=find("Disk:System:Clarice Data:bin:ls");memcpy(fs[core].fork[0],"bad core",8);memcpy(fs[tool].fork[0],"bad tool",8);memcpy(baseline,fs,sizeof fs);
 char launch[256];boundaries=0;assert(!recovery_restore(current,launch));assert(!strcmp(launch,"Disk:Clarice"));assert(!memcmp(fs[find(launch)].fork[0],"old core",8));assert(!memcmp(fs[find("Disk:System:Clarice Data:bin:ls")].fork[0],"old tool",8));int restore_steps=boundaries;closed_all();
 for(int fault=1;fault<=restore_steps;fault++){
  reset();fail_at=fault;(void)recovery_restore(current,launch);fail_at=0;closed_all();RecoverySnapshot s;assert(!recovery_inventory(current,&s,0));assert(!recovery_verify(current,&s));recovery_dispose(&s);closed_all();int retry=recovery_restore(current,launch);if(retry){fprintf(stderr,"restore retry failed boundary=%d error=%d\n",fault,retry);return 1;}closed_all();
 }
 /* Allocation exhaustion, cancellation and disk-full returned errors leave
    the immutable snapshot intact and are retryable after the resource returns. */
 reset();fail_alloc=1;assert(recovery_restore(current,launch)==memFullErr);fail_alloc=0;closed_all();assert(!recovery_restore(current,launch));closed_all();
 reset();cancel_poll=1;assert(recovery_restore(current,launch)==-128);cancel_poll=0;closed_all();assert(!recovery_restore(current,launch));closed_all();
 for(int fault=1;fault<=restore_steps;fault++){
  reset();fail_at=fault;fault_code=-34;(void)recovery_restore(current,launch);fail_at=0;closed_all();assert(!recovery_restore(current,launch));closed_all();
 }
 reset();char shown[256];assert(!recovery_member(current,"@helper",shown));
 unsigned char *finder=(unsigned char*)&fs[find(shown)].info.hFileInfo.ioFlFndrInfo;
 finder[8]^=1;finder[10]=23;finder[14]=7;
 unsigned char *extra=(unsigned char*)&fs[find(shown)].info.hFileInfo.ioFlXFndrInfo;extra[0]^=0x53;extra[1]^=0xdd;
 RecoverySnapshot visible;assert(!recovery_inventory(current,&visible,0));assert(!recovery_verify(current,&visible));
 finder[8]^=0x40;assert(recovery_verify(current,&visible)==paramErr);finder[8]^=0x40;
 extra[8]^=1;assert(recovery_verify(current,&visible)==paramErr);extra[8]^=1;
 recovery_dispose(&visible);assert(!recovery_restore(current,launch));closed_all();
 reset();char snapcore[256];assert(!recovery_member(current,"@core",snapcore));fs[find(snapcore)].fork[0][0]^=1;assert(recovery_restore(current,launch)==paramErr);assert(!memcmp(fs[find("Disk:Clarice")].fork[0],"bad core",8));closed_all();
 reset();RecoverySnapshot bad;assert(!recovery_inventory(current,&bad,0));bad.header.build++;assert(recovery_verify(current,&bad)==paramErr);recovery_dispose(&bad);closed_all();
 int crash_failures=0;
 for(int fault=1;fault<=restore_steps;fault++){
  reset();crash_at=fault;if(!setjmp(reset_point))(void)recovery_restore(current,launch);process_reset();
  RecoverySnapshot s;assert(!recovery_inventory(current,&s,0));assert(!recovery_verify(current,&s));recovery_dispose(&s);
  int retry=recovery_restore(current,launch);if(retry){crash_failures++;fprintf(stderr,"restore interruption retry blocked boundary=%d error=%d\n",fault,retry);}closed_all();
 }
 /* Removal after a command rename is bound to the coherent snapshot and
    requires a usable replacement. Retry after deletion is harmless. */
 reset();assert(!recovery_restore(current,launch));RecoverySnapshot renamed;assert(!recovery_inventory(current,&renamed,0));
 int old=find("Disk:System:Clarice Data:bin:ls");
 /* Restore changed catalog identity; cleanup must refuse that changed file. */
 assert(recovery_retire_tool(&renamed,"ls","listing",validate_tool)==paramErr);
 for(unsigned i=0;i<renamed.header.count;i++)if(!strcmp(renamed.files[i].name,"ls"))renamed.files[i].source_id=fs[old].info.hFileInfo.ioDirID;
 assert(recovery_retire_tool(&renamed,"ls","listing",validate_tool)==fnfErr);assert(find("Disk:System:Clarice Data:bin:ls")>=0);
 file("Disk:System:Clarice Data:bin:listing","new tool");
 fs[old].fork[0][0]^=1;assert(recovery_retire_tool(&renamed,"ls","listing",validate_tool)==paramErr);fs[old].fork[0][0]^=1;
 assert(!recovery_retire_tool(&renamed,"ls","listing",validate_tool));assert(find("Disk:System:Clarice Data:bin:ls")<0);assert(!recovery_retire_tool(&renamed,"ls","listing",validate_tool));assert(!recovery_verify(current,&renamed));recovery_dispose(&renamed);closed_all();
 puts("PASS renamed-command cleanup: missing replacement, changed identity/content, retry and retained snapshot");
 assert(!crash_failures);
 printf("Interrupted restore: %d of %d boundaries require intervention; immutable snapshot intact\n",crash_failures,restore_steps);
 printf("PASS production native rotation %d failure boundaries; restore %d failure boundaries; both forks and snapshot survive\n",total,restore_steps);
}
