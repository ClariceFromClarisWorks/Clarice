#ifndef CLX_HOST_TEST
/* Multiversal declares KillIO but does not supply its 68K glue. */
static pascal OSErr module_kill_io(short ref){ParamBlockRec pb;memset(&pb,0,sizeof pb);pb.ioParam.ioRefNum=ref;return PBKillIOSync(&pb);}
extern unsigned long asm_fnv1a(unsigned long,const unsigned char *,unsigned long);
#endif
/* CLX1: bounded, relocatable native commands, independently loaded per run. */
static unsigned long clx_u32(const unsigned char *p){return ((unsigned long)p[0]<<24)|((unsigned long)p[1]<<16)|((unsigned long)p[2]<<8)|p[3];}
static void clx_put32(unsigned char *p,unsigned long v){p[0]=v>>24;p[1]=v>>16;p[2]=v>>8;p[3]=v;}
#ifndef CLX_HOST_TEST
extern long __mulsi3(long,long);
extern long long __muldi3(long long,long long);
extern unsigned long __udivsi3(unsigned long,unsigned long);
extern unsigned long __umodsi3(unsigned long,unsigned long);
static void *clx_import(const char *name){
#define EXPORT(symbol) if(!strcmp(name,#symbol))return (void*)symbol
#define DATA(symbol) if(!strcmp(name,#symbol))return (void*)&symbol
 EXPORT(app_pager_begin);EXPORT(app_pager_end);EXPORT(cli_mac_time);EXPORT(cli_native_create);EXPORT(cli_native_delete);EXPORT(cli_native_mkdir);EXPORT(cli_native_open);EXPORT(cli_native_close);EXPORT(cli_native_read);EXPORT(cli_native_write);EXPORT(cli_native_set);EXPORT(cli_native_lock);EXPORT(cli_native_nth);EXPORT(cli_native_rename);EXPORT(cli_native_move);EXPORT(link_target);EXPORT(protected_path);
 EXPORT(app_screen_open);EXPORT(app_screen_close);EXPORT(app_screen_rows);EXPORT(app_screen_pending);EXPORT(app_screen_flush_input);
 EXPORT(app_serial_input);EXPORT(app_serial_key);
 EXPORT(cli_environment);EXPORT(cli_path_kind);EXPORT(cli_unix_time);
 EXPORT(setlocale);EXPORT(mbrtowc);EXPORT(mbsinit);EXPORT(wcrtomb);EXPORT(btowc);EXPORT(__locale_mb_cur_max);EXPORT(nl_langinfo);
 EXPORT(memcmp);EXPORT(strtoll);EXPORT(strtoul);EXPORT(atof);EXPORT(atoi);EXPORT(strtod);EXPORT(strncpy);EXPORT(getc);EXPORT(putc);EXPORT(wctomb);EXPORT(vsnprintf);
 EXPORT(isalnum);EXPORT(isalpha);EXPORT(iscntrl);EXPORT(isdigit);EXPORT(isgraph);EXPORT(islower);EXPORT(isprint);EXPORT(ispunct);EXPORT(isspace);EXPORT(isupper);EXPORT(isxdigit);EXPORT(tolower);EXPORT(toupper);
 EXPORT(cli_ticks);EXPORT(cli_isatty);EXPORT(memchr);EXPORT(bsearch);EXPORT(memmove);EXPORT(fwrite);EXPORT(strcat);EXPORT(strtol);EXPORT(strpbrk);EXPORT(strcspn);EXPORT(ungetc);
 EXPORT(app_activity_interactive);
 EXPORT(cli_port_stats);EXPORT(cli_alloc);EXPORT(cli_free);
 EXPORT(cli_file_size);EXPORT(cli_file_seek);EXPORT(cli_stream_size);EXPORT(cli_pause_ticks);EXPORT(cli_temp_create);EXPORT(cli_remove);EXPORT(cli_file_open);EXPORT(cli_file_read);EXPORT(cli_file_write);EXPORT(cli_file_close);
 EXPORT(funopen);EXPORT(setvbuf);EXPORT(__errno);EXPORT(longjmp);EXPORT(setjmp);
 EXPORT(malloc);EXPORT(free);EXPORT(__mulsi3);EXPORT(fmemopen);EXPORT(fclose);EXPORT(fgetc);EXPORT(fputs);
 EXPORT(strcasecmp);EXPORT(strncasecmp);EXPORT(strstr);EXPORT(strncmp);EXPORT(strnlen);EXPORT(strerror);
 EXPORT(sscanf);EXPORT(sprintf);EXPORT(fprintf);EXPORT(vfprintf);EXPORT(fputc);EXPORT(fflush);
 DATA(_ctype_);EXPORT(mbtowc);EXPORT(wctype);EXPORT(iswlower);EXPORT(towupper);EXPORT(iswupper);EXPORT(towlower);EXPORT(qsort);EXPORT(iswctype);EXPORT(iswalnum);
 EXPORT(cli_read);EXPORT(cli_write);EXPORT(errsay);DATA(output_ref);DATA(input_ref);DATA(output_error);DATA(script_depth);
 EXPORT(strcmp);EXPORT(emit);EXPORT(error);EXPORT(open_path);EXPORT(app_yield);EXPORT(say);
 EXPORT(term_present);EXPORT(app_page);EXPORT(is_link);EXPORT(format_size);EXPORT(memcpy);EXPORT(strlen);
 EXPORT(memset);EXPORT(strcpy);EXPORT(snprintf);EXPORT(app_columns);EXPORT(resolve_ex);EXPORT(app_page_begin);
 EXPORT(info);EXPORT(resolve);EXPORT(encode_name);EXPORT(volume_name);EXPORT(parent_path);EXPORT(strrchr);
 EXPORT(app_output_batch);EXPORT(strchr);EXPORT(same_name);EXPORT(__muldi3);EXPORT(__udivsi3);EXPORT(__umodsi3);EXPORT(app_profile);
 if(!strcmp(name,"OPENDRIVER"))return (void*)OpenDriver;if(!strcmp(name,"CLOSEDRIVER"))return (void*)CloseDriver;
 if(!strcmp(name,"KILLIO"))return (void*)module_kill_io;if(!strcmp(name,"SERRESET"))return (void*)SerReset;
 if(!strcmp(name,"SERSETBUF"))return (void*)SerSetBuf;if(!strcmp(name,"SERGETBUF"))return (void*)SerGetBuf;
 if(!strcmp(name,"SERHSHAKE"))return (void*)SerHShake;if(!strcmp(name,"FSWRITE"))return (void*)FSWrite;
 if(!strcmp(name,"FSREAD"))return (void*)FSRead;if(!strcmp(name,"FSCLOSE"))return (void*)FSClose;
#undef DATA
#undef EXPORT
 return 0;
}
#endif
/* Runtime 1.0: native 68000 CLX calling convention and explicit import table.
   This capability does not claim POSIX semantics or a public libc ABI. */
#define CLX_RUNTIME_MAJOR 1
#define CLX_RUNTIME_MINOR 0
#define CLX_CAPABILITIES 1UL
/* Startup precedes history/preferences initialisation. Service system tasks,
   but do not dispatch shell/UI commands into a partially initialised shell. */
static int command_installing;
static int module_poll(void){if(command_installing){SystemTask();return 0;}return app_yield();}
static unsigned long module_read_ticks,module_validate_ticks,module_import_ticks,module_reloc_ticks,module_prepare_ticks;
static unsigned long clx_hash(unsigned long hash,const unsigned char *p,unsigned long n){return asm_fnv1a(hash,p,n);}
static int module_load(const char *path,int argc,char **argv,int execute){
 unsigned long started=TickCount(),phase=started;
 if(execute){port_peak=port_remaining=module_bytes=module_load_ticks=0;module_read_ticks=module_validate_ticks=module_import_ticks=module_reloc_ticks=module_prepare_ticks=0;}
 short ref=-1;unsigned char header[48];long length=32,total=0;unsigned char *file=0,*meta=0;int status=126,executed=0;
 unsigned long reading=0,validation=0,imports_time=0,relocations=0,preparation=0;
 const char *reason="damaged or truncated CLX image";
 if(open_path(path,fsRdPerm,&ref)){ref=-1;reason="cannot open installed command";goto close;}
 int e=FSRead(ref,&length,header);if(e||length!=32||GetEOF(ref,&total))goto close;
 unsigned long version=clx_u32(header+4),header_size=32;
 if(memcmp(header,"CLX1",4))goto close;
 if(version!=1&&version!=2){reason="unsupported CLX format version";goto close;}
 if(version==2){length=16;e=FSRead(ref,&length,header+32);if(e||length!=16)goto close;header_size=48;}
 reading+=TickCount()-phase;phase=TickCount();
 unsigned long size=clx_u32(header+8),entry=clx_u32(header+12),nr=clx_u32(header+16),ni=clx_u32(header+20);
 if(clx_u32(header+28)||size<4||size>262144||entry>=size||(entry&1)||nr>8192||ni>128||total!=(long)(header_size+size+nr*12+ni*64))goto close;
 if(version==2){
  if(clx_hash(2166136261UL,header,44)!=clx_u32(header+44))goto close;
  if(clx_u32(header+32)!=CLX_RUNTIME_MAJOR||clx_u32(header+36)>CLX_RUNTIME_MINOR){reason="incompatible runtime requirement (host is 1.0)";goto close;}
  if(clx_u32(header+40)&~CLX_CAPABILITIES){reason="required runtime capabilities unavailable";goto close;}
 }
 validation+=TickCount()-phase;phase=TickCount();
 if(execute&&cli_prepare_stdio()){reason="insufficient stdio memory";goto close;}
 if(execute)module_bytes=size;
 file=(unsigned char*)NewPtr(size);if(!file){reason="insufficient application memory";goto close;}
 unsigned long extra=nr*12+ni*64;meta=(unsigned char*)NewPtr(extra?extra:1);if(!meta){reason="insufficient relocation memory";goto close;}
 preparation+=TickCount()-phase;phase=TickCount();
 length=size;e=FSRead(ref,&length,file);if(e||length!=(long)size)goto close;
 length=extra;if(extra){e=FSRead(ref,&length,meta);if(e||length!=(long)extra)goto close;}
 reading+=TickCount()-phase;phase=TickCount();
 unsigned long checksum=2166136261UL;for(int part=0;part<2;part++){unsigned char *bytes=part?meta:file;unsigned long count=part?extra:size;for(unsigned long i=0;i<count;){unsigned long n=count-i;if(n>1024)n=1024;checksum=clx_hash(checksum,bytes+i,n);i+=n;if(module_poll()){status=130;goto close;}}}if(checksum!=clx_u32(header+24)){reason="CLX payload checksum mismatch";goto close;}
 validation+=TickCount()-phase;phase=TickCount();
 unsigned char *records=meta,*names=records+nr*12;void *imports[128];
 for(unsigned long i=0;i<ni;i++){char *name=(char*)names+i*64;if(!memchr(name,0,64)||!*name)goto close;imports[i]=clx_import(name);if(!imports[i]){errsay("command: unavailable runtime symbol ");errsay(name);errsay("\n");reason="incompatible runtime imports";goto close;}}
 imports_time+=TickCount()-phase;phase=TickCount();
 unsigned long previous=0;
 for(unsigned long i=0;i<nr;i++){unsigned char *r=records+i*12;unsigned long at=clx_u32(r),kind=clx_u32(r+4),target=clx_u32(r+8);if((at&1)||at>size-4||(i&&at<previous+4))goto close;previous=at;if(kind==1){if(target>size)goto close;}else if(kind==2){if(target>=ni)goto close;}else goto close;}
 if(execute)for(unsigned long i=0;i<nr;i++){unsigned char *r=records+i*12;unsigned long at=clx_u32(r),kind=clx_u32(r+4),target=clx_u32(r+8);clx_put32(file+at,kind==1?(unsigned long)file+target:(unsigned long)imports[target]+clx_u32(file+at));}
 relocations+=TickCount()-phase;phase=TickCount();
 DisposePtr((Ptr)meta);meta=0;e=FSClose(ref);ref=-1;if(e){reason="command close failed";goto close;}
 preparation+=TickCount()-phase;
 if(!execute){status=0;goto close;}
 executed=1;module_load_ticks=TickCount()-started;
 module_read_ticks=reading;module_validate_ticks=validation;module_import_ticks=imports_time;module_reloc_ticks=relocations;module_prepare_ticks=preparation;
 status=((int (*)(int,char**))(file+entry))(argc,argv)&255;
close:
 if(ref>=0)FSClose(ref);if(meta)DisposePtr((Ptr)meta);if(file)DisposePtr((Ptr)file);
 if(status==126&&!executed){errsay("command: ");errsay(path);errsay(": ");errsay(reason);errsay("\n");}
 if(execute){app_screen_close();app_activity_interactive(0);app_serial_input(0);module_ticks=TickCount()-started;}
 return status;
}
static int module_run(const char *path,int argc,char **argv){return module_load(path,argc,argv,1);}
#include "command-install.inc"

static int install_commands(void){
 int result=0;command_installing=1;
 for(int i=0;bundled_commands[i].name;i++){
  /* Existing commands are not read, rewritten or relabelled at startup. */
  char dest[PATH_CAP],stage[PATH_CAP],backup[PATH_CAP];CInfoPBRec pb;
  int e=command_paths(bundled_commands[i].name,dest,stage,backup);
  if(!e){e=info(dest,&pb);if(!e)continue;if(e==fnfErr)e=command_bundle(bundled_commands[i].name,bundled_commands[i].resource,0);}
  if(e){errsay("Command installation: ");errsay(bundled_commands[i].name);errsay("\n");if(!result)result=e;}
 }
 command_installing=0;return result;
}
