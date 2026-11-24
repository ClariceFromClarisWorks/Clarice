#ifndef CLX_HOST_TEST
/* Multiversal declares KillIO but does not supply its 68K glue. */
static pascal OSErr module_kill_io(short ref){ParamBlockRec pb;memset(&pb,0,sizeof pb);pb.ioParam.ioRefNum=ref;return PBKillIOSync(&pb);}
extern unsigned long asm_fnv1a(unsigned long,const unsigned char *,unsigned long);
#endif
/* CLX1: bounded, relocatable native commands, independently loaded per run. */
/* Loader-owned headers, NewPtr blocks and validated relocation sites are even.
   68000 move.l needs word alignment, not four-byte alignment. */
static unsigned long clx_u32(const unsigned char *p){
#ifdef __m68k__
 unsigned long v;__asm__("move.l (%1),%0":"=d"(v):"a"(p):"cc","memory");return v;
#else
 return ((unsigned long)p[0]<<24)|((unsigned long)p[1]<<16)|((unsigned long)p[2]<<8)|p[3];
#endif
}
static void clx_put32(unsigned char *p,unsigned long v){
#ifdef __m68k__
 __asm__ volatile("move.l %0,(%1)"::"d"(v),"a"(p):"cc","memory");
#else
 p[0]=v>>24;p[1]=v>>16;p[2]=v>>8;p[3]=v;
#endif
}
#ifndef CLX_HOST_TEST
extern long __mulsi3(long,long);
extern long long __muldi3(long long,long long);
extern unsigned long __udivsi3(unsigned long,unsigned long);
extern unsigned long __umodsi3(unsigned long,unsigned long);
/* A fixed sorted export table replaces a linear chain of string comparisons.
   No heap allocation or invocation cache; names and ABI bindings are unchanged. */
static int cli_appcheck(const char *);
static int cli_appinstall(const char *,const char *,unsigned long);
static void *clx_import(const char *name){
 static const struct {const char *name;void *address;} exports[]={
#define CLX_IMPORT(name,symbol) {name,(void*)(symbol)},
#include "clx-imports.inc"
#undef CLX_IMPORT
 };
 unsigned low=0,high=sizeof exports/sizeof exports[0];
 while(low<high){unsigned mid=low+(high-low)/2;int order=strcmp(name,exports[mid].name);
  if(!order)return exports[mid].address;
  if(order<0)high=mid;else low=mid+1;
 }
 return 0;
}
#endif
/* Runtime 1.0: native 68000 CLX calling convention and explicit import table.
   This capability does not claim POSIX semantics or a public libc ABI. */
#define CLX_RUNTIME_MAJOR 1
#define CLX_RUNTIME_MINOR 11
#define CLX_CAPABILITIES 1UL
/* Loading and explicit installation use the normal cancellable event path. */
static unsigned long module_poll_ticks;
static int module_poll(void){unsigned long t=TickCount();int result;result=app_yield();module_poll_ticks+=TickCount()-t;return result;}
static unsigned long module_read_ticks,module_validate_ticks,module_import_ticks,module_reloc_ticks,module_prepare_ticks;
static unsigned long clx_hash(unsigned long hash,const unsigned char *p,unsigned long n){return asm_fnv1a(hash,p,n);}
static unsigned long module_header_ticks,module_record_ticks,module_payload_ticks,module_metadata_ticks,module_structure_ticks;
static int module_fast,clx_fast_blocked;
static int module_native_error;
#include "clx-record.inc"
/* Full verification uses bounded reads, never an executable allocation.
   Separate stack frame: ordinary command launches do not reserve this buffer. */
static __attribute__((noinline)) int module_verify_payload(short ref,unsigned long size,unsigned long nr,unsigned long ni,unsigned long expected,unsigned long *metadata_hash,const char **reason){
  unsigned char block[2048] __attribute__((aligned(2)));unsigned long checksum=2166136261UL,previous=0;long length;int e;
  *metadata_hash=2166136261UL;
  for(unsigned long left=size;left;){length=left>sizeof block?sizeof block:left;long want=length;
   e=FSRead(ref,&length,block);if(e||length!=want){return e?e:ioErr;}
   checksum=clx_hash(checksum,block,length);left-=length;
   if(module_poll()){return userCanceledErr;}
  }
  for(unsigned long i=0;i<nr;){
   unsigned long count=nr-i;if(count>sizeof block/12)count=sizeof block/12;
   length=count*12;long want=length;e=FSRead(ref,&length,block);if(e||length!=want){return e?e:ioErr;}
   checksum=clx_hash(checksum,block,length);*metadata_hash=clx_hash(*metadata_hash,block,length);
   for(unsigned long j=0;j<count;j++,i++){unsigned char *r=block+j*12;
    unsigned long at=clx_u32(r),kind=clx_u32(r+4),target=clx_u32(r+8);
    if((at&1)||at>size-4||(i&&at<previous+4))return paramErr;previous=at;
    if(kind==1){if(target>size)return paramErr;}else if(kind==2){if(target>=ni)return paramErr;}else return paramErr;
   }
   if(module_poll()){return userCanceledErr;}
  }
  for(unsigned long i=0;i<ni;){
   unsigned long count=ni-i;if(count>sizeof block/64)count=sizeof block/64;
   length=count*64;long want=length;e=FSRead(ref,&length,block);if(e||length!=want){return e?e:ioErr;}
   checksum=clx_hash(checksum,block,length);*metadata_hash=clx_hash(*metadata_hash,block,length);
   for(unsigned long j=0;j<count;j++,i++){char *name=(char*)block+j*64;
    if(!memchr(name,0,64)||!*name)return paramErr;
    if(!clx_import(name)){*reason="incompatible runtime imports";return paramErr;}
   }
   if(module_poll()){return userCanceledErr;}
  }
  if(checksum!=expected){*reason="CLX payload checksum mismatch";return paramErr;}
 return 0;
}
static int module_load_source(const char *path,int argc,char **argv,int execute,unsigned char *embedded,long embedded_size){
 unsigned long started=TickCount(),phase=started;module_poll_ticks=0;
#ifdef CLARICE_NANO_PERF
 if(execute)perf_allocs=perf_free_ticks=perf_new_ticks=perf_read_ticks=perf_reads=perf_stdio_ticks=perf_image_ticks=perf_meta_ticks=0;
#endif
 if(execute){port_peak=port_remaining=module_bytes=module_load_ticks=0;module_read_ticks=module_validate_ticks=module_import_ticks=module_reloc_ticks=module_prepare_ticks=0;}
 short ref=-1;unsigned char header[48] __attribute__((aligned(2)))={0};long length=32,total=0;unsigned char *file=0,*meta=0;int status=126,executed=0;char **command_argv=0;
 unsigned long reading=0,validation=0,imports_time=0,relocations=0,preparation=0;
 char native[PATH_CAP],receipt_path[PATH_CAP];
 unsigned char identity[32],receipt[CLX_RECORD_BYTES] __attribute__((aligned(2)));
 int eligible=0,matched=0,fast=0;
 unsigned long record_time=0,payload_time=0,metadata_time=0,header_time=0,structure_time=0,metadata_hash=0;
 const char *reason="damaged or truncated CLX image";
 unsigned long version=0,header_size=32;int e=0;
 if(embedded){
  total=embedded_size;if(total<32)goto close;memcpy(header,embedded,32);
 }else{
  if((e=open_path(path,fsRdPerm,&ref))){ref=-1;reason="cannot open installed command";goto close;}
  e=FSRead(ref,&length,header);if(e||length!=32)goto close;if((e=GetEOF(ref,&total)))goto close;
 }
 version=clx_u32(header+4);
 if(memcmp(header,"CLX1",4))goto close;
 if(version!=1&&version!=2){reason="unsupported CLX format version";goto close;}
 if(version==2){
  header_size=48;if(total<48)goto close;
  if(embedded)memcpy(header+32,embedded+32,16);
  else{length=16;e=FSRead(ref,&length,header+32);if(e||length!=16)goto close;}
 }
 reading+=TickCount()-phase;phase=TickCount();
 unsigned long size=clx_u32(header+8),entry=clx_u32(header+12),nr=clx_u32(header+16),ni=clx_u32(header+20);
 if(clx_u32(header+28)||size<4||size>262144||entry>=size||(entry&1)||nr>8192||ni>128||total!=(long)(header_size+size+nr*12+ni*64))goto close;
 if(version==2){
  if(clx_hash(2166136261UL,header,44)!=clx_u32(header+44))goto close;
  if(clx_u32(header+32)!=CLX_RUNTIME_MAJOR||clx_u32(header+36)>CLX_RUNTIME_MINOR){reason="incompatible runtime requirement (host is 1.10)";goto close;}
  if(clx_u32(header+40)&~CLX_CAPABILITIES){reason="required runtime capabilities unavailable";goto close;}
 }
 header_time=TickCount()-phase;validation+=header_time;phase=TickCount();
 if(!embedded&&!resolve(path,native)&&clx_record_path(native,receipt_path)&&clx_identity(ref,native,identity)){
  eligible=1;
  if(clx_receipt_read(receipt_path,receipt)&&!memcmp(receipt+8,identity,32)&&!memcmp(receipt+40,header,48)&&memchr(receipt+88,0,256)&&!strcmp((char*)receipt+88,native))matched=1;
 }
 record_time+=TickCount()-phase;phase=TickCount();
 const char *policy=env_get("CLX_VERIFY");fast=execute&&matched&&!clx_fast_blocked&&(!*policy||!strcmp(policy,"installed"));
 int stream_check=!execute&&!embedded;
#ifdef CLARICE_VERIFY_DIAGNOSTICS
 if(stream_check&&strstr(path,"nano-profile")){ClariceMemory m;char report[220];cli_memory(&m,sizeof m);snprintf(report,sizeof report,"VERIFY memory: image=%lu metadata=%lu free=%lu largest=%lu purge=%lu contiguous=%lu legacy=%d\n",size,nr*12+ni*64,(unsigned long)m.free_bytes,(unsigned long)m.largest,(unsigned long)m.after_purge,(unsigned long)m.largest_after_purge,!!*env_get("CLX_TEST_LEGACY_CHECK"));errsay(report);}
 if(*env_get("CLX_TEST_LEGACY_CHECK"))stream_check=0;
#endif
 if(stream_check){
  e=module_verify_payload(ref,size,nr,ni,clx_u32(header+24),&metadata_hash,&reason);
  if(e){if(e==userCanceledErr)status=130;goto close;}goto verified;
 }
 if(execute&&cli_prepare_stdio()){reason="insufficient stdio memory";goto close;}
 if(execute)module_bytes=size;
#ifdef CLARICE_NANO_PERF
 unsigned long alloc_started=TickCount();
#endif
 file=embedded?embedded+header_size:(unsigned char*)NewPtr(size);
#ifdef CLARICE_NANO_PERF
 perf_image_ticks=TickCount()-alloc_started;alloc_started=TickCount();
#endif
if(!file){e=memFullErr;reason="insufficient application memory";goto close;}
 unsigned long extra=nr*12+ni*64;meta=embedded?file+size:(unsigned char*)NewPtr(extra?extra:1);
#ifdef CLARICE_NANO_PERF
 perf_meta_ticks=TickCount()-alloc_started;
#endif
if(!meta){e=memFullErr;reason="insufficient relocation memory";goto close;}
 preparation+=TickCount()-phase;phase=TickCount();
 if(!embedded){
  length=size;e=FSRead(ref,&length,file);if(e||length!=(long)size)goto close;
  length=extra;if(extra){e=FSRead(ref,&length,meta);if(e||length!=(long)extra)goto close;}
 }
 reading+=TickCount()-phase;phase=TickCount();
 if(!fast){
  unsigned long checksum=2166136261UL;for(int part=0;part<2;part++){unsigned char *bytes=part?meta:file;unsigned long count=part?extra:size;for(unsigned long i=0;i<count;){unsigned long n=count-i;if(n>4096)n=4096;checksum=clx_hash(checksum,bytes+i,n);i+=n;if(module_poll()){status=130;goto close;}}}
  payload_time=TickCount()-phase;
  if(checksum!=clx_u32(header+24)){reason="CLX payload checksum mismatch";goto close;}
 }
 phase=TickCount();
 /* Installed-fast trusts matching identity/receipt for contents, including
    relocation metadata. Bounds/import checks below still run every time.
    Strict/check/install paths always verify the complete image. */
 if(fast&&module_poll()){status=130;goto close;}
 if(eligible&&!matched){
  metadata_hash=2166136261UL;
  for(unsigned long i=0;i<extra;){unsigned long n=extra-i;if(n>4096)n=4096;metadata_hash=clx_hash(metadata_hash,meta+i,n);i+=n;if(module_poll()){status=130;goto close;}}
 }
 metadata_time=TickCount()-phase;validation+=payload_time+metadata_time;phase=TickCount();
 unsigned char *records=meta,*names=records+nr*12;void *imports[128];
 for(unsigned long i=0;i<ni;i++){char *name=(char*)names+i*64;if(!memchr(name,0,64)||!*name)goto close;imports[i]=clx_import(name);if(!imports[i]){errsay("command: unavailable runtime symbol ");errsay(name);errsay("\n");reason="incompatible runtime imports";goto close;}}
 imports_time+=TickCount()-phase;phase=TickCount();
 unsigned long previous=0;
 for(unsigned long i=0;i<nr;i++){unsigned char *r=records+i*12;unsigned long at=clx_u32(r),kind=clx_u32(r+4),target=clx_u32(r+8);if((at&1)||at>size-4||(i&&at<previous+4))goto close;previous=at;if(kind==1){if(target>size)goto close;}else if(kind==2){if(target>=ni)goto close;}else goto close;}
 structure_time=TickCount()-phase;
 if(execute)for(unsigned long i=0;i<nr;i++){unsigned char *r=records+i*12;unsigned long at=clx_u32(r),kind=clx_u32(r+4),target=clx_u32(r+8);clx_put32(file+at,kind==1?(unsigned long)file+target:(unsigned long)imports[target]+clx_u32(file+at));}
 relocations+=TickCount()-phase;phase=TickCount();
verified:
 if(eligible){unsigned char current[32];if(!clx_identity(ref,native,current)||memcmp(identity,current,32)){reason="installed command changed during validation";goto close;}}
 if(!embedded){if(meta)DisposePtr((Ptr)meta);e=FSClose(ref);ref=-1;if(e){reason="command close failed";goto close;}}meta=0;
 preparation+=TickCount()-phase;phase=TickCount();
 if(eligible&&!matched){memset(receipt,0,sizeof receipt);memcpy(receipt,"CLXV0001",8);memcpy(receipt+8,identity,32);memcpy(receipt+40,header,48);strcpy((char*)receipt+88,native);clx_put32(receipt+344,metadata_hash);clx_put32(receipt+348,clx_hash(2166136261UL,receipt,348));clx_receipt_write(receipt_path,receipt);}
 record_time+=TickCount()-phase;
 if(!execute){status=0;goto close;}
 /* Programs may reorder/replace argv entries (upstream awk does). Preserve
    the caller's pointer ownership; strings remain writable as required by C. */
 phase=TickCount();
 if(argc<0||(unsigned long)argc>0x7fffffffUL/sizeof(char*)-1){reason="invalid argument count";goto close;}
 command_argv=(char**)NewPtr(((unsigned long)argc+1)*sizeof(char*));
 if(!command_argv){e=memFullErr;reason="insufficient argument memory";goto close;}
 if(argc)memcpy(command_argv,argv,(unsigned long)argc*sizeof(char*));command_argv[argc]=0;
 preparation+=TickCount()-phase;
 executed=1;module_load_ticks=TickCount()-started;
 module_read_ticks=reading;module_validate_ticks=validation;module_import_ticks=imports_time;module_reloc_ticks=relocations;module_prepare_ticks=preparation;
#ifdef CLX_HOST_TEST
 status=clx_test_execute(file,size,argc,command_argv);
#else
 service_loading=!shell_session_active();service_registered=0;
 status=((int (*)(int,char**))(file+entry))(argc,command_argv)&255;
 service_loading=0;
 if(service_registered){/* Registration succeeds only after native setup completes. */
  if(service_registered==2)serial_service_image=(Ptr)file;else service_image=(Ptr)file;file=0;service_registered=0;
 }
#endif
close:
 module_fast=fast;module_header_ticks=header_time;module_record_ticks=record_time;module_payload_ticks=payload_time;module_metadata_ticks=metadata_time;module_structure_ticks=structure_time;
 if(command_argv)DisposePtr((Ptr)command_argv);if(ref>=0){int closed=FSClose(ref);if(!e)e=closed;}if(meta&&!embedded)DisposePtr((Ptr)meta);if(file&&!embedded)DisposePtr((Ptr)file);
 if(status==126&&!executed&&!embedded&&!resolve(path,native)){
  if(clx_receipt_forget(native)){clx_fast_blocked=1;errsay("command: cannot revoke verification record; fast launch disabled for this session\n");}
 }
 if(status==126&&!executed){errsay("command: ");errsay(path);errsay(": ");errsay(reason);errsay("\n");}
 if(execute){
#ifndef CLX_HOST_TEST
 cli_replace_abort(active_save);
#endif
 app_screen_close();app_activity_interactive(0);app_serial_input(0);module_ticks=TickCount()-started;}
 module_native_error=status==130?userCanceledErr:status==126&&!executed?(e?e:paramErr):0;
 return status;
}
static int module_load(const char *path,int argc,char **argv,int execute){return module_load_source(path,argc,argv,execute,0,0);}
static int module_run(const char *path,int argc,char **argv){return module_load(path,argc,argv,1);}
#include "command-install.inc"
