static unsigned long cli_ticks(void){return TickCount();}
static int cli_isatty(int fd){short ref=fd==0?input_ref:fd==1?output_ref:fd==2?error_ref:-2;return ref<0&&ref!=-2;}
static unsigned long port_peak,port_remaining,module_ticks,module_bytes,module_load_ticks;
static void cli_port_stats(unsigned long peak,unsigned long remaining){port_peak=peak;port_remaining=remaining;}
/* Keep error reporting, cancellation and file cleanup viable under pressure.
   No reserved block or permanent arena; the stack has its own launch reserve. */
#ifdef CLARICE_NANO_PERF
static unsigned long perf_allocs,perf_free_ticks,perf_new_ticks,perf_read_ticks,perf_reads,perf_stdio_ticks,perf_image_ticks,perf_meta_ticks;
#endif
#include "heap-admission.h"
static void *cli_alloc(unsigned long n){
#ifdef CLARICE_NANO_PERF
 unsigned long t=TickCount();perf_allocs++;
#endif
 int room=native_heap_room(n,32768L,1);
#ifdef CLARICE_NANO_PERF
 perf_free_ticks+=TickCount()-t;t=TickCount();
#endif
 if(!room){errno=ENOMEM;return NULL;}
 void *p=NewPtr(n);
#ifdef CLARICE_NANO_PERF
 perf_new_ticks+=TickCount()-t;
#endif
 if(!p)errno=ENOMEM;return p;
}
static void cli_free(void *p){if(p)DisposePtr((Ptr)p);}
/* Data-fork I/O for portable command stdio. Standard descriptors stay separate. */
static int port_errno(int e){
 switch(e){case fnfErr:case dirNFErr:errno=ENOENT;break;case permErr:case wPrErr:case fLckdErr:errno=EACCES;break;case dskFulErr:errno=ENOSPC;break;case tmfoErr:errno=EMFILE;break;case memFullErr:errno=ENOMEM;break;default:errno=EIO;break;}return -1;
}
static int cli_file_open(const char *path,const char *mode){
 short ref;char p[PATH_CAP],canonical[PATH_CAP];unsigned char name[256];int e;
 if(!mode||!mode[0]||!strchr("rwaxc",mode[0])){errno=EINVAL;return -1;}
 int binary=0,update=0;for(const char *m=mode+1;*m;m++){
  if(*m=='b'&&!binary)binary=1;else if(*m=='+'&&!update)update=1;else{errno=EINVAL;return -1;}
 }
 e=resolve_ex(path,p,canonical,1);if(e)return port_errno(e);
 int kind=vstream_kind(canonical);if(kind){if(mode[0]=='x'){errno=EEXIST;return -1;}return VSTREAM_TAG+kind*4+(update?3:mode[0]=='r'?1:2);}
 pstr(name,p);
 short permission=strchr(mode,'+')?fsRdWrPerm:mode[0]=='r'?fsRdPerm:fsWrPerm;
 if(mode[0]=='x'){e=HCreate(0,0,name,'CLRC','TEXT');if(e==dupFNErr){errno=EEXIST;return -1;}if(e)return port_errno(e);e=HOpen(0,0,name,permission,&ref);}
 else {e=HOpen(0,0,name,permission,&ref);if(e==fnfErr&&mode[0]!='r'){e=HCreate(0,0,name,'CLRC','TEXT');if(!e)e=HOpen(0,0,name,permission,&ref);}}
 if(e)return port_errno(e);
 if(mode[0]=='w')e=SetEOF(ref,0);else if(mode[0]=='a')e=SetFPos(ref,fsFromLEOF,0);
 if(e){FSClose(ref);return port_errno(e);}return ref;
}
static long cli_file_read(int ref,void *buf,unsigned long count){if(vstream_code(ref))return vstream_read(ref,buf,count);long n=count;
#ifdef CLARICE_NANO_PERF
 unsigned long t=TickCount();perf_reads++;
#endif
 int e=FSRead(ref,&n,buf);
#ifdef CLARICE_NANO_PERF
 perf_read_ticks+=TickCount()-t;
#endif
if(e&&e!=eofErr)return port_errno(e);return n;}
static long cli_file_write(int ref,const void *buf,unsigned long count){if(vstream_code(ref))return vstream_write(ref,count);long n=count;int e=FSWrite(ref,&n,buf);if(e)return port_errno(e);return n;}
static int cli_file_close(int ref){if(vstream_code(ref))return 0;int e=FSClose(ref);return e?port_errno(e):0;}
/* newlib retains FILE records for reuse. Allocate that bounded pool before any
   command image, so a late FILE allocation cannot split the freed image hole. */
static int cli_empty_read(void *cookie,char *buf,int n){(void)cookie;(void)buf;(void)n;return 0;}
static int cli_prepare_stdio(void){
 static int ready;FILE *slots[12];int count=0;
 if(ready)return 0;
#ifdef CLARICE_NANO_PERF
 unsigned long t=TickCount();
#endif
 while(count<12){slots[count]=funopen(0,cli_empty_read,0,0,0);if(!slots[count])break;count++;}
 int ok=count==12;while(count)fclose(slots[--count]);ready=ok;
#ifdef CLARICE_NANO_PERF
 perf_stdio_ticks+=TickCount()-t;
#endif
 return ok?0:-1;
}

static int cli_environment(int index,char *out,unsigned long capacity){
 const char *name=0,*value=0;int at=0;if(index<0)return 0;
 for(int i=0;i<2;i++)if(sh_env_defaults&(1<<i)){
  const char *key=i?"PWD":"HOME";struct sh_env *e=env_find(sh_env,key);
  if(at++==index){name=key;value=e&&(e->flags&(SH_ENV_SET|SH_ENV_EXPORT))==(SH_ENV_SET|SH_ENV_EXPORT)?e->value:i?unix_cwd:"/";break;}
 }
 if(!name)for(struct sh_env *e=sh_env;e;e=e->next)if((e->flags&(SH_ENV_SET|SH_ENV_EXPORT))==(SH_ENV_SET|SH_ENV_EXPORT)){
  if((!strcmp(e->name,"HOME")&&(sh_env_defaults&1))||(!strcmp(e->name,"PWD")&&(sh_env_defaults&2)))continue;
  if(at++==index){name=e->name;value=e->value;break;}
 }
 if(!name)return 0;if(strlen(name)+strlen(value)+2>capacity)return -1;
 snprintf(out,capacity,"%s=%s",name,value);return 1;
}

static int cli_path_kind(const char *path){char native[PATH_CAP];CInfoPBRec pb;int e=resolve(path,native);if(!e)e=info(native,&pb);if(e)return port_errno(e);return !!(pb.hFileInfo.ioFlAttrib&16);}
static unsigned long cli_unix_time(void){unsigned long now;GetDateTime(&now);return now-2082844800UL;}

static long cli_file_size(int ref){if(vstream_code(ref)){errno=ESPIPE;return -1;}long n;int e=GetEOF(ref,&n);return e?port_errno(e):n;}
static long cli_stream_size(int fd){short ref=fd==0?input_ref:fd==1?output_ref:error_ref;if(ref<0){errno=EBADF;return -1;}return cli_file_size(ref);}
static long cli_file_seek(int ref,long offset,int whence){if(vstream_code(ref)){errno=ESPIPE;return -1;}long pos;short mode=whence==SEEK_SET?fsFromStart:whence==SEEK_CUR?fsFromMark:whence==SEEK_END?fsFromLEOF:-1;if(mode<0){errno=EINVAL;return -1;}int e=SetFPos(ref,mode,offset);if(!e)e=GetFPos(ref,&pos);return e?port_errno(e):pos;}
static int cli_temp_create(char *path,unsigned long cap){
 static unsigned long sequence;char native[PATH_CAP];unsigned char name[256];int e;
 for(int attempt=0;attempt<32;attempt++){
  if(cap<64){errno=EINVAL;return -1;}
  snprintf(path,cap,"/tmp/clarice-sort-%lu-%lu",TickCount(),++sequence);
  e=resolve(path,native);if(e)return port_errno(e);pstr(name,native);
  e=HCreate(0,0,name,'CLRC','TEXT');if(!e)return 0;if(e!=dupFNErr)return port_errno(e);
 }errno=EEXIST;return -1;
}
static int cli_remove(const char *path){char native[PATH_CAP];unsigned char name[256];int e=resolve(path,native);if(!e){pstr(name,native);e=HDelete(0,0,name);}return e?port_errno(e):0;}
/* Catalog and fork primitives; policy/utility algorithms live in /bin. */
static unsigned long cli_mac_time(void){unsigned long n;GetDateTime(&n);return n;}
static int cli_native_create(const char *path,unsigned long type,unsigned long creator){unsigned char p[256];pstr(p,path);return HCreate(0,0,p,creator,type);}
static int cli_native_delete(const char *path){unsigned char p[256];pstr(p,path);return HDelete(0,0,p);}
static int cli_native_mkdir(const char *path){unsigned char p[256];long id;pstr(p,path);return create_dir(p,&id);}
static int cli_native_open(const char *path,int resource,int write,short *ref){unsigned char p[256];pstr(p,path);return resource?HOpenRF(0,0,p,write?fsWrPerm:fsRdPerm,ref):HOpen(0,0,p,write?fsWrPerm:fsRdPerm,ref);}
static int cli_native_close(short ref){return FSClose(ref);}
static int cli_native_read(short ref,void *buf,long *n){return FSRead(ref,n,buf);}
static int cli_native_write(short ref,const void *buf,long *n){return FSWrite(ref,n,buf);}
static int cli_native_set(const char *path,CInfoPBRec *pb){unsigned char p[256];pstr(p,path);pb->hFileInfo.ioNamePtr=p;pb->hFileInfo.ioVRefNum=0;pb->hFileInfo.ioDirID=0;pb->hFileInfo.ioFDirIndex=0;return PBSetCatInfoSync(pb);}
static int cli_native_lock(const char *path,int locked){unsigned char p[256];pstr(p,path);HParamBlockRec pb;memset(&pb,0,sizeof pb);pb.fileParam.ioNamePtr=p;return locked?PBHSetFLockSync(&pb):PBHRstFLockSync(&pb);}
static int cli_native_nth(const char *path,int index,char *leaf,CInfoPBRec *pb){CInfoPBRec dir;int e=info(path,&dir);unsigned char p[256];if(e)return e;if(!(dir.hFileInfo.ioFlAttrib&16))return dirNFErr;memset(pb,0,sizeof *pb);pb->hFileInfo.ioNamePtr=p;e=cli_native_volume(path,&pb->hFileInfo.ioVRefNum);if(e)return e;pb->hFileInfo.ioDirID=dir.dirInfo.ioDrDirID;pb->hFileInfo.ioFDirIndex=index;e=PBGetCatInfoSync(pb);if(!e){memcpy(leaf,p+1,p[0]);leaf[p[0]]=0;}return e;}
static int cli_native_rename(const char *oldpath,const char *newpath){CInfoPBRec pb;unsigned char oldname[256],newname[256];char parent1[PATH_CAP],parent2[PATH_CAP];strcpy(parent1,oldpath);strcpy(parent2,newpath);char *a=strrchr(parent1,':'),*b=strrchr(parent2,':');if(!a||!b)return bdNamErr;*a=0;*b=0;if(!same_name(parent1,parent2))return paramErr;int e=info(oldpath,&pb);if(e)return e;pstr(oldname,a+1);pstr(newname,b+1);short vol;e=cli_native_volume(oldpath,&vol);return e?e:HRename(vol,pb.hFileInfo.ioFlParID,oldname,newname);}
static int cli_native_move(const char *oldpath,const char *newpath){char parent[PATH_CAP];unsigned char src[256],dst[256];strcpy(parent,newpath);char *leaf=strrchr(parent,':');const char *oldleaf=strrchr(oldpath,':');if(!leaf||!oldleaf||!same_name(leaf+1,oldleaf+1))return paramErr;leaf[1]=0;pstr(src,oldpath);pstr(dst,parent);CMovePBRec pb;memset(&pb,0,sizeof pb);pb.ioNamePtr=src;pb.ioNewName=dst;return PBCatMoveSync(&pb);}

#include "memory-info.h"
int cli_memory(ClariceMemory *out,unsigned long capacity){
 if(!out||capacity<sizeof *out)return -1;
 THz zone=GetZone();long total=0,largest=0;
 out->heap_extent=(unsigned long)((char*)zone->bkLim-(char*)zone);
 out->free_bytes=FreeMem();out->largest=MaxBlock();
 /* Pinned Multiversal emits A062, which preserves the caller's A0 rather
    than returning contig. PurgeSpace needs the A0-result flag (A162).
    This is native trap glue, not an OS patch; no purge is performed. */
#ifdef __m68k__
 register long result_total __asm__("d0");register long result_largest __asm__("a0");
 __asm__ volatile(".word 0xA162":"=d"(result_total),"=a"(result_largest)::"d1","a1","cc","memory");
 total=result_total;largest=result_largest;
#else
 PurgeSpace(&total,&largest);
#endif
 out->after_purge=total;out->largest_after_purge=largest;
 return 0;
}

/* dd requires data-fork resize. HFS extension is explicitly zero-filled rather
   than exposing newly allocated disk contents. The caller owns partial output
   on failure/cancellation; this is not the editor safe-replacement interface. */
static int cli_file_resize(int ref,long size){
 if(vstream_code(ref)||size<0){errno=EINVAL;return -1;}
 long old,pos;int e=GetEOF(ref,&old);if(!e)e=GetFPos(ref,&pos);if(e)return port_errno(e);
 if(size<=old){e=SetEOF(ref,size);return e?port_errno(e):0;}
 e=SetFPos(ref,fsFromLEOF,0);if(e)return port_errno(e);
 unsigned char zero[512]={0};int cancelled=0;
 while(old<size){long n=size-old;if(n>(long)sizeof zero)n=sizeof zero;long want=n;e=FSWrite(ref,&n,zero);old+=n;if(!e&&n!=want)e=ioErr;if(e)break;if(app_yield()){cancelled=1;break;}}
 {int restore=SetFPos(ref,fsFromStart,pos);if(!e)e=restore;}
 if(cancelled){errno=EINTR;return -1;}return e?port_errno(e):0;
}
