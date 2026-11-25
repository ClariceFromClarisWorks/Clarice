/* Actual module/installer with a fault-injectable Macintosh file/resource shim.
   Host correctness tests, not timings or Macintosh filesystem durability proof. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <setjmp.h>
#define CLX_HOST_TEST 1
#define PATH_CAP 256
#define fnfErr -43
#define dupFNErr -48
#define paramErr -50
#define memFullErr -108
#define bdNamErr -37
#define ioErr -36
#define eofErr -39
#define resNotFound -192
#define userCanceledErr -128
#define fsRdPerm 1
#define fsWrPerm 2
typedef char *Ptr;
typedef char **Handle;
typedef struct {struct {int ioFlAttrib,ioFlFndrInfo,ioFlXFndrInfo,ioDirID,ioFlRLgLen;} hFileInfo;} CInfoPBRec;
static char root_path[PATH_CAP]="Disk:Clarice Data:";
static unsigned long port_peak,port_remaining,module_bytes,module_load_ticks,module_ticks;
static unsigned long ticks;
static const char *verify_policy="";
static const char *env_get(const char *name){(void)name;return verify_policy;}
static unsigned long heap_budget=0x7fffffffUL,heap_used,entry_free,hash_bytes;
static long FreeMem(void){return heap_budget-heap_used;}
static void PurgeMem(long n){(void)n;}
static int executions,execute_status,fail_alloc_at,alloc_calls;
static int clx_test_execute(const unsigned char *p,unsigned long n,int argc,char **argv){(void)p;(void)n;entry_free=FreeMem();executions++;if(argc>1){argv[1]=argv[0];argv[0]=NULL;}return execute_status;}
static int resolve(const char *p,char *out){if(!strncmp(p,"native:",7))p+=7;strcpy(out,p);return 0;}
static unsigned long TickCount(void){return ticks++;}
static char diagnostics[8192];
static void errsay(const char *s){if(strlen(diagnostics)+strlen(s)<sizeof diagnostics)strcat(diagnostics,s);}
static void say(const char *s){errsay(s);}
static void error(const char *s,int e){(void)e;errsay(s);}
static int cancel_at,yields;
static void SystemTask(void){}
static int app_yield(void){return cancel_at&&++yields==cancel_at;}
static void app_screen_close(void){}
static void app_activity_interactive(int n){(void)n;}
static void app_serial_input(int n){(void)n;}
static int cli_prepare_stdio(void){return 0;}
static int read_error;
static int allocs;
static unsigned long allocation_limit=~0UL,largest_request;
static Ptr NewPtr(unsigned long n){if(n>largest_request)largest_request=n;if(++alloc_calls==fail_alloc_at||n>allocation_limit)return NULL;if(n>heap_budget-heap_used)return NULL;size_t *p=malloc(sizeof(size_t)+n);if(!p)return NULL;*p=n;heap_used+=n;allocs++;return (Ptr)(p+1);}
static void DisposePtr(Ptr p){assert(p);allocs--;size_t *base=(size_t*)p-1;heap_used-=*base;free(base);}
static char export_names[8192];
static void *clx_import(const char *s){char key[70];snprintf(key,sizeof key,"\n%s\n",s);return !strcmp(s,"present")||strstr(export_names,key)?(void*)1:NULL;}
static unsigned long asm_fnv1a(unsigned long h,const unsigned char *p,unsigned long n){hash_bytes+=n;uint32_t v=h;while(n--)v=(v^*p++)*16777619u;return v;}
static void pstr(unsigned char *p,const char *s){assert(strlen(s)<256);p[0]=strlen(s);memcpy(p+1,s,p[0]);}
static const char *unpascal(const unsigned char *p){static char s[256];memcpy(s,p+1,p[0]);s[p[0]]=0;return s;}
#define MAXFILES 160
struct file {char name[256];unsigned char data[524288];long len,pos;int open,dir,identity,resource_fork,finder,modified;};
static struct file files[MAXFILES];
static int generation;
static int writes,fail_write,short_write,closes,fail_close,renames,fail_rename,fail_rename_again,flushes,fail_flush,crash_flush;
static jmp_buf crash;
static int find(const char *p){for(int i=0;i<MAXFILES;i++)if(*files[i].name&&!strcmp(p,files[i].name))return i;return -1;}
static int info(const char *p,CInfoPBRec *pb){int i=find(p);if(i<0)return fnfErr;pb->hFileInfo.ioFlAttrib=files[i].dir?16:0;pb->hFileInfo.ioFlFndrInfo=files[i].finder;pb->hFileInfo.ioFlXFndrInfo=0;pb->hFileInfo.ioDirID=files[i].identity;pb->hFileInfo.ioFlRLgLen=files[i].resource_fork;return 0;}
static int cli_native_set(const char *p,CInfoPBRec *pb){int i=find(p);if(i<0)return fnfErr;files[i].finder=pb->hFileInfo.ioFlFndrInfo;return 0;}
static int HCreate(int v,int d,const unsigned char *p,unsigned long creator,unsigned long type){(void)v;(void)d;(void)creator;(void)type;const char *s=unpascal(p);if(find(s)>=0)return dupFNErr;for(int i=0;i<MAXFILES;i++)if(!*files[i].name){memset(&files[i],0,sizeof files[i]);strcpy(files[i].name,s);files[i].identity=++generation;files[i].modified=1;files[i].finder=11;return 0;}return ioErr;}
static int HOpen(int v,int d,const unsigned char *p,int perm,short *ref){(void)v;(void)d;(void)perm;int i=find(unpascal(p));if(i<0)return fnfErr;files[i].pos=0;files[i].open++;*ref=i+1;return 0;}
static int open_path(const char *p,int perm,short *ref){unsigned char pn[256];if(!strncmp(p,"native:",7))p+=7;pstr(pn,p);return HOpen(0,0,pn,perm,ref);}
static int FSRead(short ref,long *n,void *out){if(read_error)return read_error;struct file *f=&files[ref-1];long want=*n;if(*n>f->len-f->pos)*n=f->len-f->pos;memcpy(out,f->data+f->pos,*n);f->pos+=*n;return *n<want?eofErr:0;}
static int FSWrite(short ref,long *n,const void *in){struct file *f=&files[ref-1];writes++;if(writes==fail_write)return ioErr;if(writes==short_write)(*n)--;assert(f->pos+*n<=524288);memcpy(f->data+f->pos,in,*n);f->pos+=*n;f->modified++;if(f->pos>f->len)f->len=f->pos;return 0;}
static int FSClose(short ref){assert(files[ref-1].open>0);files[ref-1].open--;return ++closes==fail_close?ioErr:0;}
static int SetEOF(short ref,long n){files[ref-1].len=n;files[ref-1].modified++;return 0;}
static int GetEOF(short ref,long *n){*n=files[ref-1].len;return 0;}
static int HDelete(int v,int d,const unsigned char *p){(void)v;(void)d;int i=find(unpascal(p));if(i<0)return fnfErr;assert(!files[i].open);files[i].name[0]=0;return 0;}
static int cli_native_volume(const char *p,short *v){(void)p;*v=1;return 0;}
static int FlushVol(void *p,short v){(void)p;(void)v;flushes++;if(flushes==crash_flush)longjmp(crash,1);return flushes==fail_flush?ioErr:0;}
static int cli_native_rename(const char *a,const char *b){renames++;if(renames==fail_rename||renames==fail_rename_again)return ioErr;int i=find(a);if(i<0)return fnfErr;if(find(b)>=0)return dupFNErr;strcpy(files[i].name,b);return 0;}
static unsigned char bundle[256];static long bundle_len;static char *bundle_ptr=(char*)bundle;
static int cli_native_delete(const char *p){unsigned char n[256];pstr(n,p);return HDelete(0,0,n);}
static Handle Get1Resource(unsigned long type,short id){(void)type;(void)id;return &bundle_ptr;}
static long GetHandleSize(Handle h){(void)h;return bundle_len;}
static void HLock(Handle h){(void)h;}
static void HUnlock(Handle h){(void)h;}
static void ReleaseResource(Handle h){(void)h;}
static int clx_identity(short ref,const char *native,unsigned char out[32]){
 int i=find(native);if(i<0||ref!=i+1)return 0;memset(out,0,32);
 uint32_t values[]={123,(uint32_t)files[i].identity,45,1,(uint32_t)files[i].modified,(uint32_t)files[i].len,(uint32_t)files[i].resource_fork,0};
 for(int j=0;j<8;j++){uint32_t x=values[j];out[j*4]=x>>24;out[j*4+1]=x>>16;out[j*4+2]=x>>8;out[j*4+3]=x;}return 1;
}
#include "../../src/module.inc"
static int cases;
static void reset(void){assert(!allocs);for(int i=0;i<MAXFILES;i++)assert(!files[i].open);memset(files,0,sizeof files);diagnostics[0]=0;writes=closes=renames=flushes=yields=0;fail_write=short_write=fail_close=fail_rename=fail_rename_again=fail_flush=crash_flush=cancel_at=0;verify_policy="";executions=0;clx_fast_blocked=0;cases++;}
static long make_image(unsigned char *b,int version,int value){memset(b,0,256);memcpy(b,"CLX1",4);clx_put32(b+4,version);clx_put32(b+8,4);int h=version==2?48:32;b[h]=value;clx_put32(b+24,asm_fnv1a(2166136261UL,b+h,4));if(version==2){clx_put32(b+32,1);clx_put32(b+40,1);clx_put32(b+44,asm_fnv1a(2166136261UL,b,44));}return h+4;}
static int put(const char *path,const unsigned char *data,long len){unsigned char pn[256];pstr(pn,path);assert(!HCreate(0,0,pn,0,0));int i=find(path);memcpy(files[i].data,data,len);files[i].len=len;files[i].resource_fork=567;files[i].finder=123;return i;}
static const char *dest="Disk:Clarice Data:bin:grep",*old="Disk:Clarice Data:bin:.clx-grep.old",*stage="Disk:Clarice Data:bin:.clx-grep.new",*source="Disk:upgrade";
static unsigned char updated[256];static long updated_len;
static int setup(void){int i=put(dest,bundle,bundle_len);put(source,updated,updated_len);return i;}
static void same_old(int i,const char *path){assert(find(path)==i);assert(files[i].resource_fork==567&&files[i].finder==123);assert(files[i].len==bundle_len&&!memcmp(files[i].data,bundle,bundle_len));}
static int install_test_command(void){
 if(find(source)<0)put(source,bundle,bundle_len);
 return command_replace("grep",source,0);
}
static void collection_fixture(void){
 int d=put("Kit",(const unsigned char*)"",0);files[d].dir=1;
 for(int j=0;standard_commands[j];j++){char p[256];snprintf(p,sizeof p,"Kit:%s",standard_commands[j]);put(p,bundle,bundle_len);}
}
int main(int argc,char **argv){
 if(argc>1&&!strcmp(argv[1],"--pressure")){
  unsigned char *image=calloc(1,65536+48);assert(image);
  memcpy(image,"CLX1",4);clx_put32(image+4,2);clx_put32(image+8,65536);clx_put32(image+32,1);clx_put32(image+40,1);
  clx_put32(image+24,asm_fnv1a(2166136261UL,image+48,65536));clx_put32(image+44,asm_fnv1a(2166136261UL,image,44));
  put(source,image,65536+48);free(image);heap_budget=73728;
  int rc=module_run(source,0,NULL);printf("pressure: status=%d executions=%d entry-free=%lu remaining=%lu\n",rc,executions,entry_free,heap_used);assert(!heap_used&&rc==126&&!executions&&module_native_error==memFullErr);
  heap_budget=131072;allocation_limit=32768;
  assert(module_run(source,0,NULL)==126&&!executions&&!heap_used);
  allocation_limit=~0UL;cancel_at=1;assert(module_run(source,0,NULL)==130&&!executions&&!heap_used);
  cancel_at=0;assert(!module_run(source,0,NULL)&&entry_free>=32768&&!heap_used);return 0;
 }
 if(argc>1){FILE *exports=fopen(argv[1],"rb");assert(exports);size_t n=fread(export_names,1,sizeof export_names-1,exports);export_names[n]=0;fclose(exports);
  for(int a=2;a<argc;a++){reset();FILE *f=fopen(argv[a],"rb");assert(f);strcpy(files[0].name,dest);files[0].len=fread(files[0].data,1,sizeof files[0].data,f);assert(!ferror(f));fclose(f);allocation_limit=16384;alloc_calls=0;int status=command_check_native(dest);assert(!alloc_calls);if(status)fprintf(stderr,"%s: %s",argv[a],diagnostics);assert(!status);assert(!allocs&&!files[0].open);}
  printf("PASS: production loader validated %d actual command images and imports (no execution)\n",argc-2);return 0;
 }
 bundle_len=make_image(bundle,1,1);updated_len=make_image(updated,2,2);
 /* Core-owned updater image: same full validation, no file/receipt dependency.
    Caller owns the locked resource and releases it after return. */
 {unsigned char embedded[4096];reset();memcpy(embedded,bundle,bundle_len);
  assert(module_load_source("built-in clarice",0,NULL,1,embedded,bundle_len)==0);
  assert(executions==1&&!allocs);
  reset();memcpy(embedded,bundle,bundle_len);embedded[bundle_len-1]^=1;
  assert(module_load_source("built-in clarice",0,NULL,1,embedded,bundle_len)==126);
  assert(!executions&&!allocs);
  reset();assert(module_load_source("built-in clarice",0,NULL,1,embedded,31)==126);
  assert(!executions&&!allocs);
  reset();memcpy(embedded,bundle,bundle_len);cancel_at=1;
  assert(module_load_source("built-in clarice",0,NULL,1,embedded,bundle_len)==130);
  assert(!executions&&!allocs);
  reset();memcpy(embedded,bundle,bundle_len);alloc_calls=0;fail_alloc_at=1;
  assert(module_load_source("built-in clarice",0,NULL,1,embedded,bundle_len)==126);
  assert(!executions&&!allocs);fail_alloc_at=0;
 }

 reset();setup();allocation_limit=0;largest_request=0;
 assert(module_run(dest,0,0)==126&&module_native_error==memFullErr);
 assert(strstr(diagnostics,"insufficient application memory")&&largest_request==4);
 alloc_calls=0;assert(!cli_appcheck(dest)&&!alloc_calls&&!allocs);
 allocation_limit=~0UL;reset();assert(cli_appcheck("missing")==fnfErr);
 reset();setup();fail_close=1;assert(cli_appcheck(dest)==ioErr);
 reset();setup();fail_close=1;assert(module_run(dest,0,0)==126);assert(module_native_error==ioErr&&!allocs&&!executions);fail_close=0;assert(!module_run(dest,0,0)&&!allocs);
 reset();setup();read_error=ioErr;assert(cli_appcheck(dest)==ioErr);read_error=0;assert(!cli_appcheck(dest));
 reset();setup();char first[]="awk",program[]="BEGIN {}";char *args[]={first,program,NULL};
 for(int run=0;run<3;run++){execute_status=run==0?0:run==1?2:130;assert(module_run(dest,2,args)==execute_status);assert(args[0]==first&&args[1]==program&&!args[2]);assert(!allocs);}execute_status=0;
 alloc_calls=0;fail_alloc_at=3;int prior=executions;assert(module_run(dest,2,args)==126);assert(executions==prior&&!allocs);assert(strstr(diagnostics,"argument memory"));fail_alloc_at=0;
 assert(!module_run(dest,2,args));assert(!allocs);

 reset();assert(!install_test_command());assert(find(dest)>=0);int count=writes;assert(!install_test_command());assert(writes==count);assert(!command_check_native(dest));
 reset();int i=setup();assert(!install_test_command());same_old(i,dest);
 reset();i=put(dest,updated,updated_len);assert(!install_test_command());assert(find(dest)==i&&!memcmp(files[i].data,updated,updated_len));
 reset();i=put(dest,(const unsigned char*)"damaged",7);assert(!install_test_command());assert(files[i].len==7);assert(command_check_native(dest)==126);assert(strstr(diagnostics,"damaged"));
 reset();unsigned char bad[256];memcpy(bad,updated,updated_len);clx_put32(bad+36,CLX_RUNTIME_MINOR+1);clx_put32(bad+44,asm_fnv1a(2166136261UL,bad,44));i=put(dest,bad,updated_len);assert(!install_test_command());assert(command_check_native(dest)==126);assert(strstr(diagnostics,"host is " CLX_STRING(CLX_RUNTIME_MAJOR) "." CLX_STRING(CLX_RUNTIME_MINOR)));assert(!memcmp(files[i].data,bad,updated_len));
 reset();memcpy(bad,updated,updated_len);clx_put32(bad+36,CLX_RUNTIME_MINOR);clx_put32(bad+44,asm_fnv1a(2166136261UL,bad,44));put(dest,bad,updated_len);assert(!command_check_native(dest));
 reset();memcpy(bad,updated,updated_len);clx_put32(bad+40,2);clx_put32(bad+44,asm_fnv1a(2166136261UL,bad,44));put(dest,bad,updated_len);assert(command_check_native(dest)==126);assert(strstr(diagnostics,"capabilities"));
 reset();memcpy(bad,updated,updated_len);bad[48]^=1;put(dest,bad,updated_len);assert(command_check_native(dest)==126);assert(strstr(diagnostics,"checksum"));
 reset();i=setup();assert(!command_replace("grep",source,1));same_old(i,old);assert(!command_check_native(dest));int installed=find(dest);assert(files[installed].finder==123);assert(!install_test_command());assert(find(dest)==installed);assert(command_replace("grep",source,1)==dupFNErr);assert(!command_recover("grep"));same_old(i,dest);assert(find(stage)==installed);
 reset();i=setup();assert(!command_replace("grep",source,1));same_old(i,old);assert(!command_check_native(dest));
 reset();i=setup();files[find(source)].data[0]=0;assert(command_replace("grep",source,1));same_old(i,dest);assert(find(stage)<0&&find(old)<0);
 for(int f=0;f<8;f++){reset();i=setup();switch(f){case 0:fail_write=1;break;case 1:short_write=1;break;case 2:fail_close=1;break;case 3:fail_flush=1;break;case 4:fail_flush=2;break;case 5:fail_rename=1;break;case 6:fail_rename=2;break;case 7:cancel_at=1;break;}assert(command_replace("grep",source,1));same_old(i,dest);assert(find(stage)<0);}
 reset();i=setup();fail_rename=2;fail_rename_again=3;assert(command_replace("grep",source,1));same_old(i,old);assert(find(dest)<0);assert(install_test_command());assert(find(dest)<0);assert(!command_recover("grep"));same_old(i,dest);
 reset();i=setup();fail_flush=3;assert(command_replace("grep",source,1));same_old(i,old);assert(!command_check_native(dest));
 for(int point=1;point<=3;point++){reset();i=setup();crash_flush=point;if(!setjmp(crash)){command_replace("grep",source,1);assert(0);}crash_flush=0;assert(!allocs);if(point==1)same_old(i,dest);else same_old(i,old);if(point==2){assert(install_test_command());assert(find(dest)<0);assert(!command_recover("grep"));same_old(i,dest);}if(point==3){assert(!command_recover("grep"));same_old(i,dest);}}

 for(int fault=0;fault<9;fault++){reset();memcpy(bad,updated,updated_len);
  switch(fault){case 0:bad[44]^=1;break;case 1:clx_put32(bad+4,99);break;
   case 2:clx_put32(bad+28,1);break;case 3:clx_put32(bad+8,2);break;
   case 4:clx_put32(bad+12,3);break;case 5:clx_put32(bad+16,8193);break;
   case 6:clx_put32(bad+20,129);break;case 7:clx_put32(bad+32,2);clx_put32(bad+44,asm_fnv1a(2166136261UL,bad,44));break;
   case 8:break;
  }put(dest,bad,updated_len-(fault==8));assert(command_check_native(dest)==126);
 }
 /* Malformed relocation/import metadata cannot reach relocation application. */
 for(int fault=0;fault<5;fault++){reset();long len=make_image(bad,1,1);clx_put32(bad+16,1);clx_put32(bad+36,fault==0?1:0);clx_put32(bad+40,fault==1?3:2);clx_put32(bad+44,fault==2?1:0);clx_put32(bad+20,1);memcpy(bad+48,fault==3?"missing":"present",8);if(fault==4)memset(bad+48,'x',64);len=112;clx_put32(bad+24,asm_fnv1a(2166136261UL,bad+32,len-32));put(dest,bad,len);assert(command_check_native(dest)==126);}
 reset();i=setup();cancel_at=2;assert(command_replace("grep",source,1)==userCanceledErr);same_old(i,dest);
 reset();i=setup();put(stage,(const unsigned char*)"unrelated",9);assert(command_replace("grep",source,1)==dupFNErr);same_old(i,dest);assert(files[find(stage)].len==9);
 reset();i=setup();files[i].dir=1;assert(command_replace("grep",source,1));same_old(i,dest);
 reset();assert(command_replace("../grep",source,1)==bdNamErr);assert(command_replace("this-command-name-is-over-twenty",source,1)==bdNamErr);

 /* Receipt digest is optional; full payload and structural checks never are. */
 reset();make_image(bad,1,1);clx_put32(bad+16,1);clx_put32(bad+20,1);
 clx_put32(bad+36,0);clx_put32(bad+40,2);clx_put32(bad+44,0);memcpy(bad+48,"present",8);
 clx_put32(bad+24,asm_fnv1a(2166136261UL,bad+32,80));i=put(source,bad,112);
 for(int receipt_needed=0;receipt_needed<2;receipt_needed++){
  short ref;assert(!open_path(source,fsRdPerm,&ref));files[i].pos=32;
  unsigned long digest=0;const char *reason="test";hash_bytes=0;
  assert(!module_verify_payload(ref,4,1,1,clx_u32(bad+24),receipt_needed?&digest:NULL,&reason));
  assert(hash_bytes==(unsigned long)(receipt_needed?156:80));assert(!FSClose(ref));
  if(receipt_needed)assert(digest==asm_fnv1a(2166136261UL,bad+36,76));
 }
 files[i].data[32]^=1;assert(command_check_native(source)==126);
 /* Fast policy: the host callback stands in for executing validated 68000 code. */
 reset();i=put(dest,updated,updated_len);assert(!module_run(dest,0,0)&&!module_fast);assert(!module_run(dest,0,0)&&module_fast);assert(executions==2);
 char receipt_name[PATH_CAP];assert(clx_record_path(dest,receipt_name));int rec=find(receipt_name);assert(rec>=0&&files[rec].len==CLX_RECORD_BYTES);
 assert(!install_test_command());assert(!module_run(dest,0,0)&&module_fast);
 verify_policy="strict";assert(!module_run(dest,0,0)&&!module_fast);verify_policy="typo";assert(!module_run(dest,0,0)&&!module_fast);verify_policy="";
 assert(!command_check_native(dest)&&!module_fast);assert(!module_run(dest,0,0)&&module_fast);
 /* Deliberately demonstrate the documented undetectable-content-change risk. */
 files[i].data[48]^=1;assert(!module_run(dest,0,0)&&module_fast);
 verify_policy="strict";assert(module_run(dest,0,0)==126);assert(find(receipt_name)<0);verify_policy="";assert(module_run(dest,0,0)==126);
 reset();i=put(dest,updated,updated_len);assert(!command_check_native(dest));files[i].modified++;files[i].data[48]^=1;assert(module_run(dest,0,0)==126);
 reset();i=put(dest,updated,updated_len);assert(!command_check_native(dest));files[i].identity++;assert(!module_run(dest,0,0)&&!module_fast);assert(!module_run(dest,0,0)&&module_fast);
 reset();i=put(source,updated,updated_len);assert(!module_run(source,0,0)&&!module_fast);assert(!module_run(source,0,0)&&!module_fast);
 reset();i=put(dest,updated,updated_len);assert(!command_check_native(dest));rec=find(receipt_name);files[rec].data[100]^=1;assert(!module_run(dest,0,0)&&!module_fast);
 reset();i=put(dest,updated,updated_len);assert(!command_check_native(dest));files[i].len--;assert(module_run(dest,0,0)==126&&executions==0);assert(find(receipt_name)<0);
 reset();i=setup();assert(!command_check_native(dest));assert(!command_replace("grep",source,1));assert(!module_run(dest,0,0)&&module_fast);assert(!command_recover("grep"));assert(find(receipt_name)<0);assert(!module_run(dest,0,0)&&!module_fast);assert(!module_run(dest,0,0)&&module_fast);
 reset();i=setup();assert(!command_check_native(dest));assert(!command_replace("grep",source,1));assert(!module_run(dest,0,0)&&module_fast);
 reset();i=put(dest,updated,updated_len);fail_write=1;assert(!command_check_native(dest));assert(!module_run(dest,0,0)&&!module_fast);assert(!module_run(dest,0,0)&&!module_fast); /* torn record remains untrusted */
 reset();i=setup();assert(!command_check_native(dest));fail_rename=2;assert(command_replace("grep",source,1));same_old(i,dest);assert(find(receipt_name)<0);assert(!module_run(dest,0,0)&&!module_fast);
 /* Structural bounds remain mandatory even with a matching receipt. */
 for(int fault=0;fault<3;fault++){
  reset();make_image(bad,1,1);clx_put32(bad+16,1);clx_put32(bad+36,0);clx_put32(bad+40,1);clx_put32(bad+44,0);long len=48;clx_put32(bad+24,asm_fnv1a(2166136261UL,bad+32,16));i=put(dest,bad,len);assert(!command_check_native(dest));rec=find(receipt_name);
  clx_put32(files[i].data+36,fault==0?2:8);
  if(fault==2){clx_put32(files[rec].data+344,asm_fnv1a(2166136261UL,files[i].data+36,12));clx_put32(files[rec].data+348,asm_fnv1a(2166136261UL,files[rec].data,348));}
  assert(module_run(dest,0,0)==126&&executions==0);
 }

 reset();i=put(dest,updated,updated_len);assert(!command_check_native(dest));files[i].modified++;assert(!module_run(dest,0,0)&&!module_fast);assert(!module_run(dest,0,0)&&module_fast);
 reset();i=put(dest,updated,updated_len);assert(!command_check_native(dest));files[i].data[48]^=1;clx_put32(files[i].data+24,asm_fnv1a(2166136261UL,files[i].data+48,4));clx_put32(files[i].data+44,asm_fnv1a(2166136261UL,files[i].data,44));assert(!module_run(dest,0,0)&&!module_fast);assert(!module_run(dest,0,0)&&module_fast);
 reset();i=put(dest,updated,updated_len);assert(!command_check_native(dest));rec=find(receipt_name);files[rec].len=11;assert(!module_run(dest,0,0)&&!module_fast);
 reset();i=put(dest,updated,updated_len);put(receipt_name,(const unsigned char*)"user data",9);assert(!command_check_native(dest));assert(files[find(receipt_name)].len==9);assert(!module_run(dest,0,0)&&!module_fast);
 /* Valid receipt checksum cannot excuse invalid import names or symbols. */
 for(int fault=0;fault<2;fault++){
  reset();make_image(bad,1,1);clx_put32(bad+20,1);memcpy(bad+36,"present",8);long len=100;clx_put32(bad+24,asm_fnv1a(2166136261UL,bad+32,len-32));i=put(dest,bad,len);assert(!command_check_native(dest));rec=find(receipt_name);
  if(fault)memset(files[i].data+36,'x',64);else memcpy(files[i].data+36,"missing",8);
  clx_put32(files[rec].data+344,asm_fnv1a(2166136261UL,files[i].data+36,64));clx_put32(files[rec].data+348,asm_fnv1a(2166136261UL,files[rec].data,348));assert(module_run(dest,0,0)==126&&!executions);
 }
 /* In-range relocation corruption with unchanged identity is the explicitly
    accepted fast-mode risk; strict/check must reject and revoke its receipt. */
 for(int check=0;check<2;check++){
  reset();make_image(bad,1,1);clx_put32(bad+16,1);clx_put32(bad+36,0);clx_put32(bad+40,1);clx_put32(bad+44,0);clx_put32(bad+24,asm_fnv1a(2166136261UL,bad+32,16));i=put(dest,bad,48);assert(!command_check_native(dest));
  clx_put32(files[i].data+44,2);assert(!module_run(dest,0,0)&&module_fast&&executions==1);
  if(check)assert(command_check_native(dest));else{verify_policy="strict";assert(module_run(dest,0,0)==126);}
  assert(find(receipt_name)<0);verify_policy="";assert(module_run(dest,0,0)==126&&executions==1);
 }
 reset();i=put(dest,updated,updated_len);assert(!command_check_native(dest));cancel_at=1;assert(module_run(dest,0,0)==130&&!executions);cancel_at=0;assert(!module_run(dest,0,0)&&module_fast);
 reset();assert(command_setup("Missing")==fnfErr&&!writes);
 reset();collection_fixture();assert(command_setup("Kit:grep")==paramErr&&!writes);
 /* Complete external setup, rerun, missing source, cancellation and OOM. */
 reset();collection_fixture();assert(!command_setup("Kit"));int before=writes;
 assert(!command_setup("Kit")&&writes==before);assert(!allocs);
 reset();collection_fixture();i=put(dest,(const unsigned char*)"broken",6);
 assert(!command_setup("Kit"));assert(files[i].len==6&&files[i].resource_fork==567);
 reset();collection_fixture();files[find("Kit:awk")].data[0]=0;
 assert(command_setup("Kit"));assert(find("Disk:Clarice Data:bin:awk")<0);
 files[find("Kit:awk")].data[0]='C';assert(!command_setup("Kit"));
 for(int point=1;point<=4;point++){
  reset();collection_fixture();alloc_calls=0;fail_alloc_at=point;
  int e=command_setup("Kit");assert(!e&&!alloc_calls);assert(!allocs);fail_alloc_at=0;
  assert(!command_setup("Kit"));
 }
 reset();collection_fixture();cancel_at=1;assert(command_setup("Kit")==userCanceledErr);
 cancel_at=0;assert(!command_setup("Kit"));
 reset();collection_fixture();char *repair[]={"appinstall","repair","grep","Kit"};
 i=put(dest,bundle,bundle_len);assert(!appinstall_command(4,repair));same_old(i,old);
 reset();collection_fixture();char *install[]={"appinstall","install","native:Kit:grep","grep"};
 assert(!appinstall_command(4,install));before=writes;assert(!appinstall_command(4,install)&&writes==before);
 /* Repository slots are separate from user-managed appinstall backups.
    Retry of an installed identical image must not rotate another generation. */
 const char *managed_old="Disk:Clarice Data:bin:.repo-grep.old",*managed_stage="Disk:Clarice Data:bin:.repo-grep.new";
 reset();i=setup();put(old,(const unsigned char*)"manual backup",13);assert(!cli_appinstall(source,"grep",2));same_old(i,managed_old);assert(find(old)>=0);
 int previous=find(managed_old);before=writes;assert(!cli_appinstall(source,"grep",3));same_old(previous,managed_old);assert(writes==before);
 reset();i=setup();assert(!cli_native_rename(dest,managed_old));int staged=put(managed_stage,updated,updated_len);files[staged].resource_fork=0;
 assert(!cli_appinstall(source,"grep",2));same_old(i,managed_old);assert(find(dest)>=0);
 reset();i=setup();files[find(source)].data[0]=0;assert(cli_appinstall(source,"grep",2));same_old(i,dest);assert(find(managed_old)<0);
 reset();i=setup();cancel_at=1;assert(cli_appinstall(source,"grep",2));same_old(i,dest);cancel_at=0;assert(!cli_appinstall(source,"grep",2));same_old(i,managed_old);
 reset();printf("PASS: %d loader/installer scenarios; no leaked allocations or descriptors\n",cases-1);return 0;
}
