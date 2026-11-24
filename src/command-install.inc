/* Explicit, recoverable updates. All transaction names are siblings on HFS.
   Never remove a pre-existing file, including stale transaction files. */
static const char *const standard_commands[]={
#define CLARICE_COMMAND(name,image) name,
#include "standard-commands.inc"
#undef CLARICE_COMMAND
 0};
static void command_install_hint(const char *name){
 for(int i=0;standard_commands[i];i++)if(!strcmp(name,standard_commands[i])){
  errsay("Install missing standard tools with appinstall setup DIRECTORY.\nSee help appinstall.\n");break;
 }
}
static int command_paths_for(const char *name,char *dest,char *stage,char *backup,int managed){
 size_t n=strlen(name);if(!n||n>20)return bdNamErr;
 for(size_t i=0;i<n;i++)if(!((name[i]>='a'&&name[i]<='z')||(name[i]>='A'&&name[i]<='Z')||(name[i]>='0'&&name[i]<='9')||name[i]=='_'||name[i]=='-'))return bdNamErr;
 if(strlen(root_path)+n+17>=PATH_CAP||strlen(root_path)+n+17>255)return bdNamErr;
 strcpy(dest,root_path);strcat(dest,"bin:");strcat(dest,name);
 strcpy(stage,root_path);strcat(stage,managed?"bin:.repo-":"bin:.clx-");strcat(stage,name);strcat(stage,".new");
 strcpy(backup,root_path);strcat(backup,managed?"bin:.repo-":"bin:.clx-");strcat(backup,name);strcat(backup,".old");return 0;
}
static int command_paths(const char *n,char *d,char *s,char *b){return command_paths_for(n,d,s,b,0);}
static int command_absent(const char *path){CInfoPBRec pb;int e=info(path,&pb);return e==fnfErr?0:e?e:dupFNErr;}
static int command_flush(const char *path){short vol;int e=cli_native_volume(path,&vol);return e?e:FlushVol(0,vol);}
static int command_check_native(const char *path){char full[PATH_CAP+8];snprintf(full,sizeof full,"native:%s",path);return module_load(full,0,0,0);}
static void command_recovery(const char *path){errsay("appinstall: retained recovery file: native:");errsay(path);errsay("\n");}
static int command_restore(const char *backup,const char *dest){int e=cli_native_rename(backup,dest);if(!e)e=command_flush(dest);if(e)command_recovery(backup);return e;}
/* Streaming file source; no bundled resource is held during validation. */
static int command_replace_mode(const char *name,const char *source,int replace,int managed){
 char dest[PATH_CAP],stage[PATH_CAP],backup[PATH_CAP];CInfoPBRec pb;short in=-1,out=-1;
 int e=command_paths_for(name,dest,stage,backup,managed),exists=0,owned=0;if(e)return e;
 e=info(dest,&pb);if(!e){if(!replace)return 0;if(pb.hFileInfo.ioFlAttrib&16)return paramErr;exists=1;}else if(e!=fnfErr)return e;
 if((e=command_absent(stage))||(e=command_absent(backup))){
  errsay("appinstall: unfinished update or retained previous version; inspect .clx-*.new/.old before replacing\n");return e;
 }
 {e=open_path(source,fsRdPerm,&in);if(e)return e;long bytes=0;e=GetEOF(in,&bytes);
  if(e||bytes<36||bytes>368688L){FSClose(in);errsay("appinstall: invalid command file size\n");return e?e:paramErr;}
 }
 unsigned char pn[256];pstr(pn,stage);e=HCreate(0,0,pn,'CLRC','CLX1');if(e)goto done;owned=1;
 e=HOpen(0,0,pn,fsWrPerm,&out);if(e){out=-1;goto done;}
 {unsigned char buffer[4096];long offset=0;
  for(;;){long n;const unsigned char *data;
   n=sizeof buffer;e=FSRead(in,&n,buffer);if(e==eofErr)e=0;if(e)break;data=buffer;
   if(!n)break;if(n<0||offset+n>368688L){e=paramErr;break;}long want=n;e=FSWrite(out,&n,data);if(!e&&n!=want)e=ioErr;if(e)break;offset+=n;
   if(module_poll()){e=userCanceledErr;break;}
  }
 }
 if(in>=0){int closed=FSClose(in);in=-1;if(!e)e=closed;}
 if(out>=0){int closed=FSClose(out);out=-1;if(!e)e=closed;}
 if(e)goto done;
 {int valid=command_check_native(stage);if(valid){e=valid==130?userCanceledErr:paramErr;goto done;}}
 if(exists){CInfoPBRec staged;e=info(stage,&staged);if(e)goto done;
  staged.hFileInfo.ioFlFndrInfo=pb.hFileInfo.ioFlFndrInfo;
  staged.hFileInfo.ioFlXFndrInfo=pb.hFileInfo.ioFlXFndrInfo;
  if((e=cli_native_set(stage,&staged)))goto done;
 }
 if((e=command_flush(stage)))goto done;
 /* No yields between renames: cancellation cannot strand an ordinary update.
    Flush after saving the old entry makes recovery survive a later failure. */
 if((e=clx_receipt_forget(dest)))goto done;
 if(exists){e=cli_native_rename(dest,backup);if(e)goto done;
  e=command_flush(backup);if(e){command_restore(backup,dest);goto done;}}
 e=cli_native_rename(stage,dest);
 if(e){if(exists)command_restore(backup,dest);goto done;}
 owned=0;e=command_flush(dest);if(exists)command_recovery(backup);
 if(!e){int valid=command_check_native(dest);if(valid)e=valid==130?userCanceledErr:paramErr;}
done:
 if(in>=0)FSClose(in);if(out>=0)FSClose(out);
 if(owned){int cleanup=HDelete(0,0,pn);if(cleanup)command_recovery(stage);}
 return e;
}
static int command_replace(const char *n,const char *s,int r){return command_replace_mode(n,s,r,0);}
static int command_recover(const char *name){
 char dest[PATH_CAP],stage[PATH_CAP],backup[PATH_CAP];CInfoPBRec pb;int e=command_paths(name,dest,stage,backup);if(e)return e;
 e=info(backup,&pb);if(e)return e;if(pb.hFileInfo.ioFlAttrib&16)return paramErr;
 /* Recovery restores the exact old file, even if it was damaged/incompatible.
    Never execute it here, and never discard the current installed candidate. */
 e=info(dest,&pb);int exists=!e;if(e&&e!=fnfErr)return e;
 if((e=clx_receipt_forget(dest)))return e;
 if(exists){if(pb.hFileInfo.ioFlAttrib&16)return paramErr;if((e=command_absent(stage)))return e;
  if((e=cli_native_rename(dest,stage)))return e;
  if((e=command_flush(stage))){command_restore(stage,dest);return e;}}
 e=cli_native_rename(backup,dest);
 if(e){if(exists)command_restore(stage,dest);return e;}
 e=command_flush(dest);if(exists)command_recovery(stage);return e;
}
/* Explicit setup alone traverses the collection; use native source paths. */
static int command_source(const char *directory,const char *name,char *source){
 char native[PATH_CAP];CInfoPBRec pb;int e=resolve(directory,native);if(e)return e;
 e=info(native,&pb);if(e)return e;if(!(pb.hFileInfo.ioFlAttrib&16))return paramErr;
 size_t n=strlen(native),leaf=strlen(name);
 if(n+leaf+2>PATH_CAP||n+leaf+1>255)return bdNamErr;
 strcpy(source,"native:");strcat(source,native);if(n&&native[n-1]!=':')strcat(source,":");strcat(source,name);return 0;
}
static int command_setup(const char *directory){
 char source[PATH_CAP+8];int source_error=command_source(directory,"",source);
 if(source_error)return source_error;size_t base=strlen(source);
 say("appinstall: existing commands are kept unchanged, not checked.\n");
 for(int i=0;standard_commands[i];i++){
  if(module_poll())return userCanceledErr;
  char dest[PATH_CAP],stage[PATH_CAP],backup[PATH_CAP];CInfoPBRec pb;
  const char *name=standard_commands[i];int e=command_paths(name,dest,stage,backup);
  if(!e){e=info(dest,&pb);if(!e)continue;if(e==fnfErr){if(base+strlen(name)>=sizeof source||base-7+strlen(name)>255)e=bdNamErr;else{strcpy(source+base,name);e=0;}}}
  if(!e)e=command_replace(name,source,0);
  if(e){errsay("appinstall: stopped at ");errsay(name);errsay("; fix the error and rerun setup.\n");return e;}
  say("Installed ");say(name);say("\n");
 }
 return 0;
}
static int appinstall_command(int argc,char **av){
 if(argc==2&&!strcmp(av[1],"--help")){say(
  "appinstall setup DIRECTORY | install FILE NAME | check FILE\n"
  "           replace FILE NAME | repair NAME DIRECTORY | recover NAME\n"
  "setup installs the standard collection; install installs one command.\n"
  "Both leave every existing command unchanged, even if damaged.\n"
  "NAME: /bin leaf, 1..20 letters, digits, _ or -.\n"
  "check fully verifies without execution. replace/repair explicitly replace.\n"
  "repair reads DIRECTORY/NAME. recover restores .clx-NAME.old.\n"
  "Previous files retain both forks; Finder metadata is preserved.\n"
  "The replacement is data-only; old resource forks stay in the backup.\n"
  "Existing .old/.new files block replacement: inspect and move them first.\n"
  "CLX_VERIFY=strict fully verifies launches; installed uses receipts.\n"
  "Status: 0 success; 1 install error; 2 usage; 126 bad CLX; 130 cancel.\n");return 0;}
 if(argc==3&&!strcmp(av[1],"check"))return module_load(av[2],0,0,0);
 int e=paramErr;
 if(argc==4&&(!strcmp(av[1],"replace")||!strcmp(av[1],"install")))e=command_replace(av[3],av[2],!strcmp(av[1],"replace"));
 else if(argc==3&&!strcmp(av[1],"setup"))e=command_setup(av[2]);
 else if(argc==3&&!strcmp(av[1],"recover"))e=command_recover(av[2]);
 else if(argc==4&&!strcmp(av[1],"repair")){
  char source[PATH_CAP+8],dest[PATH_CAP],stage[PATH_CAP],backup[PATH_CAP];
  e=command_paths(av[2],dest,stage,backup);
  if(!e)e=command_source(av[3],av[2],source);
  if(!e)e=command_replace(av[2],source,1);
 }else{errsay("usage: appinstall --help (repair requires NAME DIRECTORY)\n");return 2;}
 if(e){error("appinstall",e);return e==userCanceledErr?130:1;}
 say("appinstall: complete (existing files kept by install/setup)\n");return 0;
}

/* Repository files use a distinct bounded slot; historical/manual appinstall
   backups are never renamed or pruned by managed updates. */
static int cli_appcheck(const char *source){int e=module_load(source,0,0,0);return e==130?userCanceledErr:e?paramErr:0;}
static int command_same_image(const char *source,const char *native){
 char path[PATH_CAP+8];snprintf(path,sizeof path,"native:%s",native);short a=-1,b=-1;int e=open_path(source,fsRdPerm,&a);if(e)return e;e=open_path(path,fsRdPerm,&b);
 long an=0,bn=0;if(!e)e=GetEOF(a,&an);if(!e)e=GetEOF(b,&bn);if(!e&&an!=bn)e=paramErr;
 unsigned char x[512],y[512];while(!e&&an){long n=an>(long)sizeof x?(long)sizeof x:an,want=n;e=FSRead(a,&n,x);if(!e&&n!=want)e=ioErr;n=want;if(!e)e=FSRead(b,&n,y);if(!e&&(n!=want||memcmp(x,y,n)))e=paramErr;an-=want;if(!e&&module_poll())e=userCanceledErr;}
 if(b>=0){int z=FSClose(b);if(!e)e=z;}int z=FSClose(a);return e?e:z;
}
static int cli_appinstall(const char *source,const char *name,unsigned long version){
 char dest[PATH_CAP],stage[PATH_CAP],backup[PATH_CAP];CInfoPBRec c;(void)version;
 int e=command_paths_for(name,dest,stage,backup,1);if(e)return e;
 e=cli_appcheck(source);if(e)return e;
 e=info(dest,&c);if(e==fnfErr){int b=info(backup,&c);if(!b)e=command_restore(backup,dest);else if(b!=fnfErr)return b;else e=0;}if(e)return e;
 /* A completed rename followed by a failed catalog append is resumable.
    The exact old backup stays until the whole release has completed. */
 int old=info(backup,&c);if(old&&old!=fnfErr)return old;
 if(!old&&!command_same_image(source,dest))return cli_appcheck(source);
 e=info(stage,&c);if(!e){if(c.hFileInfo.ioFlRLgLen)return dupFNErr;e=command_same_image(source,stage);if(e)return e;e=cli_native_delete(stage);if(e)return e;}else if(e!=fnfErr)return e;
 return command_replace_mode(name,source,1,1);
}
