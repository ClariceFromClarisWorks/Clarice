static unsigned long cli_ticks(void){return TickCount();}
static int cli_isatty(int fd){short ref=fd==0?input_ref:fd==1?output_ref:fd==2?error_ref:-2;return ref<0&&ref!=-2;}
static unsigned long port_peak,port_remaining,module_ticks,module_bytes,module_load_ticks;
static void cli_port_stats(unsigned long peak,unsigned long remaining){port_peak=peak;port_remaining=remaining;}
static void *cli_alloc(unsigned long n){return NewPtr(n);}
static void cli_free(void *p){if(p)DisposePtr((Ptr)p);}
/* Data-fork I/O for portable command stdio. Standard descriptors stay separate. */
static int port_errno(int e){
 switch(e){case fnfErr:case dirNFErr:errno=ENOENT;break;case permErr:case wPrErr:case fLckdErr:errno=EACCES;break;case dskFulErr:errno=ENOSPC;break;case tmfoErr:errno=EMFILE;break;case memFullErr:errno=ENOMEM;break;default:errno=EIO;break;}return -1;
}
static int cli_file_open(const char *path,const char *mode){
 short ref;char p[PATH_CAP];unsigned char name[256];int e;
 if(!mode||!strchr("rwa",mode[0])||!mode[0]||(mode[1]&&strcmp(mode+1,"b"))){errno=EINVAL;return -1;}
 e=resolve(path,p);if(e)return port_errno(e);pstr(name,p);
 e=HOpen(0,0,name,mode[0]=='r'?fsRdPerm:fsWrPerm,&ref);
 if(e==fnfErr&&mode[0]!='r'){e=HCreate(0,0,name,'CLRC','TEXT');if(!e)e=HOpen(0,0,name,fsWrPerm,&ref);}
 if(e)return port_errno(e);
 if(mode[0]=='w')e=SetEOF(ref,0);else if(mode[0]=='a')e=SetFPos(ref,fsFromLEOF,0);
 if(e){FSClose(ref);return port_errno(e);}return ref;
}
static long cli_file_read(int ref,void *buf,unsigned long count){long n=count;int e=FSRead(ref,&n,buf);if(e&&e!=eofErr)return port_errno(e);return n;}
static long cli_file_write(int ref,const void *buf,unsigned long count){long n=count;int e=FSWrite(ref,&n,buf);if(e)return port_errno(e);return n;}
static int cli_file_close(int ref){int e=FSClose(ref);return e?port_errno(e):0;}
/* newlib retains FILE records for reuse. Allocate that bounded pool before any
   command image, so a late FILE allocation cannot split the freed image hole. */
static int cli_empty_read(void *cookie,char *buf,int n){(void)cookie;(void)buf;(void)n;return 0;}
static int cli_prepare_stdio(void){
 static int ready;FILE *slots[12];int count=0;
 if(ready)return 0;
 while(count<12){slots[count]=funopen(0,cli_empty_read,0,0,0);if(!slots[count])break;count++;}
 int ok=count==12;while(count)fclose(slots[--count]);ready=ok;return ok?0:-1;
}

static int cli_environment(int index,char *out,unsigned long capacity){
 const char *name,*value;
 if(index==0){name="HOME";value="/";}else if(index==1){name="PWD";value=unix_cwd;}
 else if(index>=2&&index<18){name=env_names[index-2];value=env_values[index-2];if(!*name)return 0;}else return 0;
 if(strlen(name)+strlen(value)+2>capacity)return -1;snprintf(out,capacity,"%s=%s",name,value);return 1;
}
static int cli_path_kind(const char *path){char native[PATH_CAP];CInfoPBRec pb;int e=resolve(path,native);if(!e)e=info(native,&pb);if(e)return port_errno(e);return !!(pb.hFileInfo.ioFlAttrib&16);}
static unsigned long cli_unix_time(void){unsigned long now;GetDateTime(&now);return now-2082844800UL;}

static long cli_file_size(int ref){long n;int e=GetEOF(ref,&n);return e?port_errno(e):n;}
static long cli_stream_size(int fd){short ref=fd==0?input_ref:fd==1?output_ref:error_ref;if(ref<0){errno=EBADF;return -1;}return cli_file_size(ref);}
static long cli_file_seek(int ref,long offset,int whence){long pos;short mode=whence==SEEK_SET?fsFromStart:whence==SEEK_CUR?fsFromMark:whence==SEEK_END?fsFromLEOF:-1;if(mode<0){errno=EINVAL;return -1;}int e=SetFPos(ref,mode,offset);if(!e)e=GetFPos(ref,&pos);return e?port_errno(e):pos;}
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
static int cli_native_volume(const char *path,short *ref){char volume[32];const char *end=strchr(path,':');if(!end||end-path>31)return bdNamErr;memcpy(volume,path,end-path);volume[end-path]=0;for(int i=1;;i++){HParamBlockRec pb;unsigned char name[256];memset(&pb,0,sizeof pb);pb.volumeParam.ioNamePtr=name;pb.volumeParam.ioVolIndex=i;int e=PBHGetVInfoSync(&pb);if(e)return e;name[name[0]+1]=0;if(same_name(volume,(char*)name+1)){*ref=pb.volumeParam.ioVRefNum;return 0;}}}
static int cli_native_nth(const char *path,int index,char *leaf,CInfoPBRec *pb){CInfoPBRec dir;int e=info(path,&dir);unsigned char p[256];if(e)return e;if(!(dir.hFileInfo.ioFlAttrib&16))return dirNFErr;memset(pb,0,sizeof *pb);pb->hFileInfo.ioNamePtr=p;e=cli_native_volume(path,&pb->hFileInfo.ioVRefNum);if(e)return e;pb->hFileInfo.ioDirID=dir.dirInfo.ioDrDirID;pb->hFileInfo.ioFDirIndex=index;e=PBGetCatInfoSync(pb);if(!e){memcpy(leaf,p+1,p[0]);leaf[p[0]]=0;}return e;}
static int cli_native_rename(const char *oldpath,const char *newpath){CInfoPBRec pb;unsigned char oldname[256],newname[256];char parent1[PATH_CAP],parent2[PATH_CAP];strcpy(parent1,oldpath);strcpy(parent2,newpath);char *a=strrchr(parent1,':'),*b=strrchr(parent2,':');if(!a||!b)return bdNamErr;*a=0;*b=0;if(!same_name(parent1,parent2))return paramErr;int e=info(oldpath,&pb);if(e)return e;pstr(oldname,a+1);pstr(newname,b+1);short vol;e=cli_native_volume(oldpath,&vol);return e?e:HRename(vol,pb.hFileInfo.ioFlParID,oldname,newname);}
static int cli_native_move(const char *oldpath,const char *newpath){char parent[PATH_CAP];unsigned char src[256],dst[256];strcpy(parent,newpath);char *leaf=strrchr(parent,':');const char *oldleaf=strrchr(oldpath,':');if(!leaf||!oldleaf||!same_name(leaf+1,oldleaf+1))return paramErr;leaf[1]=0;pstr(src,oldpath);pstr(dst,parent);CMovePBRec pb;memset(&pb,0,sizeof pb);pb.ioNamePtr=src;pb.ioNewName=dst;return PBCatMoveSync(&pb);}
