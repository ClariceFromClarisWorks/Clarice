extern unsigned long asm_fnv1a(unsigned long,const unsigned char *,unsigned long);
/* CLX1: bounded, relocatable native commands, independently loaded per run. */
static unsigned long clx_u32(const unsigned char *p){return ((unsigned long)p[0]<<24)|((unsigned long)p[1]<<16)|((unsigned long)p[2]<<8)|p[3];}
static void clx_put32(unsigned char *p,unsigned long v){p[0]=v>>24;p[1]=v>>16;p[2]=v>>8;p[3]=v;}
extern long __mulsi3(long,long);
extern long long __muldi3(long long,long long);
extern unsigned long __udivsi3(unsigned long,unsigned long);
extern unsigned long __umodsi3(unsigned long,unsigned long);
static void *clx_import(const char *name){
#define EXPORT(symbol) if(!strcmp(name,#symbol))return (void*)symbol
#define DATA(symbol) if(!strcmp(name,#symbol))return (void*)&symbol
 EXPORT(cli_ticks);EXPORT(cli_isatty);EXPORT(memchr);EXPORT(bsearch);EXPORT(memmove);EXPORT(fwrite);EXPORT(strcat);EXPORT(strtol);EXPORT(strpbrk);EXPORT(strcspn);EXPORT(ungetc);
 EXPORT(app_activity_interactive);
 EXPORT(cli_port_stats);EXPORT(cli_alloc);EXPORT(cli_free);
 EXPORT(cli_file_open);EXPORT(cli_file_read);EXPORT(cli_file_write);EXPORT(cli_file_close);
 EXPORT(funopen);EXPORT(setvbuf);EXPORT(__errno);EXPORT(longjmp);EXPORT(setjmp);
 EXPORT(malloc);EXPORT(free);EXPORT(__mulsi3);EXPORT(fmemopen);EXPORT(fclose);EXPORT(fgetc);EXPORT(fputs);
 EXPORT(strcasecmp);EXPORT(strncasecmp);EXPORT(strstr);EXPORT(strncmp);EXPORT(strnlen);EXPORT(strerror);
 EXPORT(sprintf);EXPORT(fprintf);EXPORT(vfprintf);EXPORT(fputc);EXPORT(fflush);
 DATA(_ctype_);EXPORT(mbtowc);EXPORT(wctype);EXPORT(iswlower);EXPORT(towupper);EXPORT(iswupper);EXPORT(towlower);EXPORT(qsort);EXPORT(iswctype);EXPORT(iswalnum);
 EXPORT(cli_read);EXPORT(cli_write);EXPORT(errsay);DATA(output_ref);DATA(input_ref);DATA(output_error);DATA(script_depth);
 EXPORT(strcmp);EXPORT(emit);EXPORT(error);EXPORT(open_path);EXPORT(app_yield);EXPORT(say);
 EXPORT(term_present);EXPORT(app_page);EXPORT(is_link);EXPORT(format_size);EXPORT(memcpy);EXPORT(strlen);
 EXPORT(memset);EXPORT(strcpy);EXPORT(snprintf);EXPORT(app_columns);EXPORT(resolve_ex);EXPORT(app_page_begin);
 EXPORT(info);EXPORT(resolve);EXPORT(encode_name);EXPORT(volume_name);EXPORT(parent_path);EXPORT(strrchr);
 EXPORT(app_output_batch);EXPORT(strchr);EXPORT(same_name);EXPORT(__muldi3);EXPORT(__udivsi3);EXPORT(__umodsi3);EXPORT(app_profile);
 if(!strcmp(name,"FSREAD"))return (void*)FSRead;if(!strcmp(name,"FSCLOSE"))return (void*)FSClose;
#undef DATA
#undef EXPORT
 return 0;
}
static int module_run(const char *path,int argc,char **argv){
 unsigned long started=TickCount();port_peak=port_remaining=module_bytes=module_load_ticks=0;
 short ref;unsigned char header[32];long length=32,total=0;unsigned char *file=0,*meta=0;int status=126,executed=0;
 if(open_path(path,fsRdPerm,&ref)){error("command open",fnfErr);return 126;}
 int e=FSRead(ref,&length,header);if(e||length!=32||GetEOF(ref,&total))goto close;
 unsigned long size=clx_u32(header+8),entry=clx_u32(header+12),nr=clx_u32(header+16),ni=clx_u32(header+20);
 if(memcmp(header,"CLX1",4)||clx_u32(header+4)!=1||clx_u32(header+28)||size<2||size>262144||entry>=size||(entry&1)||nr>8192||ni>128||total!=(long)(32+size+nr*12+ni*64))goto close;
 if(cli_prepare_stdio()){errsay("command: insufficient stdio memory\n");goto close;}
 module_bytes=size;file=(unsigned char*)NewPtr(size);if(!file){errsay("command: insufficient application memory\n");goto close;}
 unsigned long extra=nr*12+ni*64;meta=(unsigned char*)NewPtr(extra?extra:1);if(!meta){errsay("command: insufficient relocation memory\n");goto close;}
 length=size;e=FSRead(ref,&length,file);if(e||length!=(long)size)goto close;
 length=extra;if(extra){e=FSRead(ref,&length,meta);if(e||length!=(long)extra)goto close;}
 unsigned long checksum=2166136261UL;for(int part=0;part<2;part++){unsigned char *bytes=part?meta:file;unsigned long count=part?extra:size;for(unsigned long i=0;i<count;){unsigned long n=count-i;if(n>1024)n=1024;checksum=asm_fnv1a(checksum,bytes+i,n);i+=n;if(app_yield()){status=130;goto close;}}}if(checksum!=clx_u32(header+24))goto close;
 unsigned char *records=meta,*names=records+nr*12;void *imports[128];
 for(unsigned long i=0;i<ni;i++){char *name=(char*)names+i*64;if(!memchr(name,0,64)||!*name)goto close;imports[i]=clx_import(name);if(!imports[i]){errsay("command: unavailable runtime symbol ");errsay(name);errsay("\n");goto close;}}
 unsigned long previous=0;
 for(unsigned long i=0;i<nr;i++){unsigned char *r=records+i*12;unsigned long at=clx_u32(r),kind=clx_u32(r+4),target=clx_u32(r+8);if((at&1)||at>size-4||(i&&at<previous+4))goto close;previous=at;if(kind==1){if(target>size)goto close;}else if(kind==2){if(target>=ni)goto close;}else goto close;}
 for(unsigned long i=0;i<nr;i++){unsigned char *r=records+i*12;unsigned long at=clx_u32(r),kind=clx_u32(r+4),target=clx_u32(r+8);clx_put32(file+at,kind==1?(unsigned long)file+target:(unsigned long)imports[target]+clx_u32(file+at));}
 DisposePtr((Ptr)meta);meta=0;FSClose(ref);ref=-1;executed=1;module_load_ticks=TickCount()-started;status=((int (*)(int,char**))(file+entry))(argc,argv)&255;
close:
 if(ref>=0)FSClose(ref);if(meta)DisposePtr((Ptr)meta);if(file)DisposePtr((Ptr)file);if(!executed&&status!=130)errsay("command: cannot execute this image\n");app_activity_interactive(0);module_ticks=TickCount()-started;return status;
}

/* Packaged command images are resources only for installation; executions read /bin. */
static int install_command_set(short resource_id,const char *const *names){
 Handle image=Get1Resource('CLX1',resource_id);if(!image)return resNotFound;
 long size=GetHandleSize(image);HLock(image);int result=0;
 for(int i=0;names[i];i++){
  char path[PATH_CAP];unsigned char name[256],head[32];CInfoPBRec pb;short ref;long n=32;
  snprintf(path,sizeof path,"%.240sbin:%s",root_path,names[i]);pstr(name,path);int e=info(path,&pb);
  if(!e){if(pb.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC'||pb.hFileInfo.ioFlFndrInfo.fdType!='CLX1')continue;
   e=HOpen(0,0,name,fsRdPerm,&ref);if(!e){e=FSRead(ref,&n,head);FSClose(ref);if(!e&&n==32&&!memcmp(head,*image,32))continue;}
  }else if(e==fnfErr)e=HCreate(0,0,name,'CLRC','CLX1');
  if(!e)e=HOpen(0,0,name,fsWrPerm,&ref);if(e){result=e;break;}
  n=size;e=FSWrite(ref,&n,*image);if(!e&&n!=size)e=ioErr;if(!e)e=SetEOF(ref,size);FSClose(ref);if(e){result=e;break;}
 }
 HUnlock(image);ReleaseResource(image);return result;
}

static int install_commands(void){
 static const char *const core[]={"ls","df","cat","benchls","benchscan",0};
 static const char *const grep[]={"grep",0};
 static const char *const sed[]={"sed",0};
 int e=install_command_set(200,core);if(!e)e=install_command_set(201,grep);return e?e:install_command_set(202,sed);
}
