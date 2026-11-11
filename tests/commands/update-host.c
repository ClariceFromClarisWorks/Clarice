/* Actual module/installer with a fault-injectable Macintosh file/resource shim.
   Host correctness tests, not timings or Macintosh filesystem durability proof. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <setjmp.h>
#define CLX_HOST_TEST 1
#define PATH_CAP 256
#define fnfErr -43
#define dupFNErr -48
#define paramErr -50
#define bdNamErr -37
#define ioErr -36
#define eofErr -39
#define resNotFound -192
#define userCanceledErr -128
#define fsRdPerm 1
#define fsWrPerm 2
typedef char *Ptr;
typedef char **Handle;
typedef struct {struct {int ioFlAttrib,ioFlFndrInfo,ioFlXFndrInfo;} hFileInfo;} CInfoPBRec;
static char root_path[PATH_CAP]="Disk:Clarice Data:";
static unsigned long port_peak,port_remaining,module_bytes,module_load_ticks,module_ticks;
static unsigned long ticks;
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
static int allocs;
static Ptr NewPtr(unsigned long n){void *p=malloc(n);if(p)allocs++;return p;}
static void DisposePtr(Ptr p){assert(p);allocs--;free(p);}
static char export_names[8192];
static void *clx_import(const char *s){char key[70];snprintf(key,sizeof key,"\n%s\n",s);return !strcmp(s,"present")||strstr(export_names,key)?(void*)1:NULL;}
static unsigned long asm_fnv1a(unsigned long h,const unsigned char *p,unsigned long n){uint32_t v=h;while(n--)v=(v^*p++)*16777619u;return v;}
static void pstr(unsigned char *p,const char *s){assert(strlen(s)<256);p[0]=strlen(s);memcpy(p+1,s,p[0]);}
static const char *unpascal(const unsigned char *p){static char s[256];memcpy(s,p+1,p[0]);s[p[0]]=0;return s;}
#define MAXFILES 96
struct file {char name[256];unsigned char data[524288];long len,pos;int open,dir,identity,resource_fork,finder;};
static struct file files[MAXFILES];
static int writes,fail_write,short_write,closes,fail_close,renames,fail_rename,fail_rename_again,flushes,fail_flush,crash_flush;
static jmp_buf crash;
static int find(const char *p){for(int i=0;i<MAXFILES;i++)if(*files[i].name&&!strcmp(p,files[i].name))return i;return -1;}
static int info(const char *p,CInfoPBRec *pb){int i=find(p);if(i<0)return fnfErr;pb->hFileInfo.ioFlAttrib=files[i].dir?16:0;pb->hFileInfo.ioFlFndrInfo=files[i].finder;pb->hFileInfo.ioFlXFndrInfo=0;return 0;}
static int cli_native_set(const char *p,CInfoPBRec *pb){int i=find(p);if(i<0)return fnfErr;files[i].finder=pb->hFileInfo.ioFlFndrInfo;return 0;}
static int HCreate(int v,int d,const unsigned char *p,unsigned long creator,unsigned long type){(void)v;(void)d;(void)creator;(void)type;const char *s=unpascal(p);if(find(s)>=0)return dupFNErr;for(int i=0;i<MAXFILES;i++)if(!*files[i].name){memset(&files[i],0,sizeof files[i]);strcpy(files[i].name,s);files[i].identity=i+1;files[i].finder=11;return 0;}return ioErr;}
static int HOpen(int v,int d,const unsigned char *p,int perm,short *ref){(void)v;(void)d;(void)perm;int i=find(unpascal(p));if(i<0)return fnfErr;files[i].pos=0;files[i].open++;*ref=i+1;return 0;}
static int open_path(const char *p,int perm,short *ref){unsigned char pn[256];if(!strncmp(p,"native:",7))p+=7;pstr(pn,p);return HOpen(0,0,pn,perm,ref);}
static int FSRead(short ref,long *n,void *out){struct file *f=&files[ref-1];long want=*n;if(*n>f->len-f->pos)*n=f->len-f->pos;memcpy(out,f->data+f->pos,*n);f->pos+=*n;return *n<want?eofErr:0;}
static int FSWrite(short ref,long *n,const void *in){struct file *f=&files[ref-1];writes++;if(writes==fail_write)return ioErr;if(writes==short_write)(*n)--;assert(f->pos+*n<=524288);memcpy(f->data+f->pos,in,*n);f->pos+=*n;if(f->pos>f->len)f->len=f->pos;return 0;}
static int FSClose(short ref){assert(files[ref-1].open>0);files[ref-1].open--;return ++closes==fail_close?ioErr:0;}
static int GetEOF(short ref,long *n){*n=files[ref-1].len;return 0;}
static int HDelete(int v,int d,const unsigned char *p){(void)v;(void)d;int i=find(unpascal(p));if(i<0)return fnfErr;assert(!files[i].open);files[i].name[0]=0;return 0;}
static int cli_native_volume(const char *p,short *v){(void)p;*v=1;return 0;}
static int FlushVol(void *p,short v){(void)p;(void)v;flushes++;if(flushes==crash_flush)longjmp(crash,1);return flushes==fail_flush?ioErr:0;}
static int cli_native_rename(const char *a,const char *b){renames++;if(renames==fail_rename||renames==fail_rename_again)return ioErr;int i=find(a);if(i<0)return fnfErr;if(find(b)>=0)return dupFNErr;strcpy(files[i].name,b);return 0;}
static unsigned char bundle[256];static long bundle_len;static char *bundle_ptr=(char*)bundle;
static Handle Get1Resource(unsigned long type,short id){(void)type;(void)id;return &bundle_ptr;}
static long GetHandleSize(Handle h){(void)h;return bundle_len;}
static void HLock(Handle h){(void)h;}
static void HUnlock(Handle h){(void)h;}
static void ReleaseResource(Handle h){(void)h;}
#include "../../src/module.inc"
static int cases;
static void reset(void){assert(!allocs);for(int i=0;i<MAXFILES;i++)assert(!files[i].open);memset(files,0,sizeof files);diagnostics[0]=0;writes=closes=renames=flushes=yields=0;fail_write=short_write=fail_close=fail_rename=fail_rename_again=fail_flush=crash_flush=cancel_at=0;cases++;}
static long make_image(unsigned char *b,int version,int value){memset(b,0,256);memcpy(b,"CLX1",4);clx_put32(b+4,version);clx_put32(b+8,4);int h=version==2?48:32;b[h]=value;clx_put32(b+24,asm_fnv1a(2166136261UL,b+h,4));if(version==2){clx_put32(b+32,1);clx_put32(b+40,1);clx_put32(b+44,asm_fnv1a(2166136261UL,b,44));}return h+4;}
static int put(const char *path,const unsigned char *data,long len){unsigned char pn[256];pstr(pn,path);assert(!HCreate(0,0,pn,0,0));int i=find(path);memcpy(files[i].data,data,len);files[i].len=len;files[i].resource_fork=567;files[i].finder=123;return i;}
static const char *dest="Disk:Clarice Data:bin:grep",*old="Disk:Clarice Data:bin:.clx-grep.old",*stage="Disk:Clarice Data:bin:.clx-grep.new",*source="Disk:upgrade";
static unsigned char updated[256];static long updated_len;
static int setup(void){int i=put(dest,bundle,bundle_len);put(source,updated,updated_len);return i;}
static void same_old(int i,const char *path){assert(find(path)==i);assert(files[i].resource_fork==567&&files[i].finder==123);assert(files[i].len==bundle_len&&!memcmp(files[i].data,bundle,bundle_len));}
int main(int argc,char **argv){
 if(argc>1){FILE *exports=fopen(argv[1],"rb");assert(exports);size_t n=fread(export_names,1,sizeof export_names-1,exports);export_names[n]=0;fclose(exports);
  for(int a=2;a<argc;a++){reset();FILE *f=fopen(argv[a],"rb");assert(f);strcpy(files[0].name,dest);files[0].len=fread(files[0].data,1,sizeof files[0].data,f);assert(!ferror(f));fclose(f);int status=command_check_native(dest);if(status)fprintf(stderr,"%s: %s",argv[a],diagnostics);assert(!status);assert(!allocs&&!files[0].open);}
  printf("PASS: production loader validated %d actual command images and imports (no execution)\n",argc-2);return 0;
 }
 bundle_len=make_image(bundle,1,1);updated_len=make_image(updated,2,2);
 reset();assert(!install_commands());assert(!yields);assert(find(dest)>=0);int count=writes;assert(!install_commands());assert(writes==count);assert(!command_check_native(dest));
 reset();int i=setup();assert(!install_commands());same_old(i,dest);
 reset();i=put(dest,updated,updated_len);assert(!install_commands());assert(find(dest)==i&&!memcmp(files[i].data,updated,updated_len));
 reset();i=put(dest,(const unsigned char*)"damaged",7);assert(!install_commands());assert(files[i].len==7);assert(command_check_native(dest)==126);assert(strstr(diagnostics,"damaged"));
 reset();unsigned char bad[256];memcpy(bad,updated,updated_len);clx_put32(bad+36,1);clx_put32(bad+44,asm_fnv1a(2166136261UL,bad,44));i=put(dest,bad,updated_len);assert(!install_commands());assert(command_check_native(dest)==126);assert(strstr(diagnostics,"incompatible runtime"));assert(!memcmp(files[i].data,bad,updated_len));
 reset();memcpy(bad,updated,updated_len);clx_put32(bad+40,2);clx_put32(bad+44,asm_fnv1a(2166136261UL,bad,44));put(dest,bad,updated_len);assert(command_check_native(dest)==126);assert(strstr(diagnostics,"capabilities"));
 reset();memcpy(bad,updated,updated_len);bad[48]^=1;put(dest,bad,updated_len);assert(command_check_native(dest)==126);assert(strstr(diagnostics,"checksum"));
 reset();i=setup();assert(!command_replace("grep",source,0,0,1));same_old(i,old);assert(!command_check_native(dest));int installed=find(dest);assert(files[installed].finder==123);assert(!install_commands());assert(find(dest)==installed);assert(command_replace("grep",source,0,0,1)==dupFNErr);assert(!command_recover("grep"));same_old(i,dest);assert(find(stage)==installed);
 reset();i=setup();assert(!command_bundle("grep",201,1));same_old(i,old);assert(!command_check_native(dest));
 reset();i=setup();files[find(source)].data[0]=0;assert(command_replace("grep",source,0,0,1));same_old(i,dest);assert(find(stage)<0&&find(old)<0);
 for(int f=0;f<8;f++){reset();i=setup();switch(f){case 0:fail_write=1;break;case 1:short_write=1;break;case 2:fail_close=1;break;case 3:fail_flush=1;break;case 4:fail_flush=2;break;case 5:fail_rename=1;break;case 6:fail_rename=2;break;case 7:cancel_at=1;break;}assert(command_replace("grep",source,0,0,1));same_old(i,dest);assert(find(stage)<0);}
 reset();i=setup();fail_rename=2;fail_rename_again=3;assert(command_replace("grep",source,0,0,1));same_old(i,old);assert(find(dest)<0);assert(install_commands());assert(find(dest)<0);assert(!command_recover("grep"));same_old(i,dest);
 reset();i=setup();fail_flush=3;assert(command_replace("grep",source,0,0,1));same_old(i,old);assert(!command_check_native(dest));
 for(int point=1;point<=3;point++){reset();i=setup();crash_flush=point;if(!setjmp(crash)){command_replace("grep",source,0,0,1);assert(0);}crash_flush=0;assert(!allocs);if(point==1)same_old(i,dest);else same_old(i,old);if(point==2){assert(install_commands());assert(find(dest)<0);assert(!command_recover("grep"));same_old(i,dest);}if(point==3){assert(!command_recover("grep"));same_old(i,dest);}}

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
 reset();i=setup();cancel_at=2;assert(command_replace("grep",source,0,0,1)==userCanceledErr);same_old(i,dest);
 reset();i=setup();put(stage,(const unsigned char*)"unrelated",9);assert(command_replace("grep",source,0,0,1)==dupFNErr);same_old(i,dest);assert(files[find(stage)].len==9);
 reset();i=setup();files[i].dir=1;assert(command_replace("grep",source,0,0,1));same_old(i,dest);
 reset();assert(command_replace("../grep",source,0,0,1)==bdNamErr);assert(command_replace("this-command-name-is-over-twenty",source,0,0,1)==bdNamErr);
 reset();printf("PASS: %d loader/installer scenarios; no leaked allocations or descriptors\n",cases-1);return 0;
}
