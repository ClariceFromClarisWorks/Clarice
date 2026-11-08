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
