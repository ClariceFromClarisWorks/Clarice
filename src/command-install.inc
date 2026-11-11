/* Explicit, recoverable updates. All transaction names are siblings on HFS.
   Never remove a pre-existing file, including stale transaction files. */
static const struct {const char *name;short resource;} bundled_commands[]={
 {"ls",200},{"df",200},{"cat",200},{"benchls",200},{"benchscan",200},
 {"grep",201},{"sed",202},{"awk",203},{"serial",204},{"wc",205},
 {"head",206},{"cut",207},{"tr",208},{"uniq",209},{"tee",210},
 {"tail",211},{"sort",212},{"cp",213},{"mv",213},{"touch",213},
 {"rmdir",213},{"mkdir",213},{"rm",213},{"ln",213},{"readlink",213},
 {"hfsattr",213},{"find",214},{"du",214},{"file",214},{"od",215},
 {"more",215},{"basename",216},{"dirname",217},{0,0}};
static int command_paths(const char *name,char *dest,char *stage,char *backup){
 size_t n=strlen(name);if(!n||n>20)return bdNamErr;
 for(size_t i=0;i<n;i++)if(!((name[i]>='a'&&name[i]<='z')||(name[i]>='A'&&name[i]<='Z')||(name[i]>='0'&&name[i]<='9')||name[i]=='_'||name[i]=='-'))return bdNamErr;
 if(strlen(root_path)+n+16>=PATH_CAP||strlen(root_path)+n+16>255)return bdNamErr;
 strcpy(dest,root_path);strcat(dest,"bin:");strcat(dest,name);
 strcpy(stage,root_path);strcat(stage,"bin:.clx-");strcat(stage,name);strcat(stage,".new");
 strcpy(backup,root_path);strcat(backup,"bin:.clx-");strcat(backup,name);strcat(backup,".old");return 0;
}
static int command_absent(const char *path){CInfoPBRec pb;int e=info(path,&pb);return e==fnfErr?0:e?e:dupFNErr;}
static int command_flush(const char *path){short vol;int e=cli_native_volume(path,&vol);return e?e:FlushVol(0,vol);}
static int command_check_native(const char *path){char full[PATH_CAP+8];snprintf(full,sizeof full,"native:%s",path);return module_load(full,0,0,0);}
static void command_recovery(const char *path){errsay("appinstall: retained recovery file: native:");errsay(path);errsay("\n");}
static int command_restore(const char *backup,const char *dest){int e=cli_native_rename(backup,dest);if(!e)e=command_flush(dest);if(e)command_recovery(backup);return e;}
/* image is a locked bundled resource, or source is an independently supplied file. */
static int command_replace(const char *name,const char *source,const unsigned char *image,long image_size,int replace){
 char dest[PATH_CAP],stage[PATH_CAP],backup[PATH_CAP];CInfoPBRec pb;short in=-1,out=-1;
 int e=command_paths(name,dest,stage,backup),exists=0,owned=0;if(e)return e;
 e=info(dest,&pb);if(!e){if(!replace)return 0;if(pb.hFileInfo.ioFlAttrib&16)return paramErr;exists=1;}else if(e!=fnfErr)return e;
 if((e=command_absent(stage))||(e=command_absent(backup))){
  errsay("appinstall: unfinished update or retained previous version; inspect .clx-*.new/.old before replacing\n");return e;
 }
 if(source){e=open_path(source,fsRdPerm,&in);if(e)return e;long bytes=0;e=GetEOF(in,&bytes);
  if(e||bytes<36||bytes>368688L){FSClose(in);errsay("appinstall: invalid command file size\n");return e?e:paramErr;}
 }else if(image_size<36||image_size>368688L)return paramErr;
 unsigned char pn[256];pstr(pn,stage);e=HCreate(0,0,pn,'CLRC','CLX1');if(e)goto done;owned=1;
 e=HOpen(0,0,pn,fsWrPerm,&out);if(e){out=-1;goto done;}
 {unsigned char buffer[4096];long offset=0;
  for(;;){long n;const unsigned char *data;
   if(source){n=sizeof buffer;e=FSRead(in,&n,buffer);if(e==eofErr)e=0;if(e)break;data=buffer;}
   else{n=image_size-offset;if(n>(long)sizeof buffer)n=sizeof buffer;data=image+offset;}
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
 if(exists){e=cli_native_rename(dest,backup);if(e)goto done;
  e=command_flush(backup);if(e){command_restore(backup,dest);goto done;}}
 e=cli_native_rename(stage,dest);
 if(e){if(exists)command_restore(backup,dest);goto done;}
 owned=0;e=command_flush(dest);if(exists)command_recovery(backup);
done:
 if(in>=0)FSClose(in);if(out>=0)FSClose(out);
 if(owned){int cleanup=HDelete(0,0,pn);if(cleanup)command_recovery(stage);}
 return e;
}
static int command_bundle(const char *name,short id,int replace){
 Handle image=Get1Resource('CLX1',id);if(!image)return resNotFound;
 long size=GetHandleSize(image);HLock(image);int e=command_replace(name,0,(const unsigned char*)*image,size,replace);
 HUnlock(image);ReleaseResource(image);return e;
}
static int command_recover(const char *name){
 char dest[PATH_CAP],stage[PATH_CAP],backup[PATH_CAP];CInfoPBRec pb;int e=command_paths(name,dest,stage,backup);if(e)return e;
 e=info(backup,&pb);if(e)return e;if(pb.hFileInfo.ioFlAttrib&16)return paramErr;
 /* Recovery restores the exact old file, even if it was damaged/incompatible.
    Never execute it here, and never discard the current installed candidate. */
 e=info(dest,&pb);int exists=!e;if(e&&e!=fnfErr)return e;
 if(exists){if(pb.hFileInfo.ioFlAttrib&16)return paramErr;if((e=command_absent(stage)))return e;
  if((e=cli_native_rename(dest,stage)))return e;
  if((e=command_flush(stage))){command_restore(stage,dest);return e;}}
 e=cli_native_rename(backup,dest);
 if(e){if(exists)command_restore(stage,dest);return e;}
 e=command_flush(dest);if(exists)command_recovery(stage);return e;
}
static int appinstall_command(int argc,char **av){
 if(argc==2&&!strcmp(av[1],"--help")){
  say("appinstall check FILE | replace FILE NAME | repair NAME | recover NAME\n"
      "NAME is a /bin leaf (1..20 letters, digits, _ or -).\n"
      "check validates without executing; replace validates before installing.\n"
      "repair explicitly installs the bundled copy; recover restores .clx-NAME.old.\n"
      "Previous files retain both forks. Existing Finder metadata is preserved.\nReplacement is a CLX data file (resource fork stays with the previous version).\n"
      "Existing .old/.new files block updates: inspect and move them before retrying.\n");return 0;
 }
 if(argc==3&&!strcmp(av[1],"check"))return module_load(av[2],0,0,0);
 int e=paramErr;
 if(argc==4&&!strcmp(av[1],"replace"))e=command_replace(av[3],av[2],0,0,1);
 else if(argc==3&&!strcmp(av[1],"recover"))e=command_recover(av[2]);
 else if(argc==3&&!strcmp(av[1],"repair")){
  e=fnfErr;for(int i=0;bundled_commands[i].name;i++)if(!strcmp(av[2],bundled_commands[i].name)){e=command_bundle(av[2],bundled_commands[i].resource,1);break;}
 }else{errsay("usage: appinstall --help\n");return 2;}
 if(e){error("appinstall",e);return e==userCanceledErr?130:1;}say("appinstall: complete\n");return 0;
}
