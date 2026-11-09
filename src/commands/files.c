/* Clarice HFS file utilities. Algorithms reside in this loadable command image. */
#include "mac-files.h"
static int recursive,preserve,force,parents,nocreate;
static unsigned long sequence;
static int fail(const char *what,int e){error(what,e);return e==130?130:1;}
static void four(unsigned long value,char out[5]){for(int i=0;i<4;i++){unsigned char c=value>>(24-i*8);out[i]=c>=32&&c<127?c:'.';}out[4]=0;}
static int code(const char *s,unsigned long *n){if(strlen(s)!=4)return 0;*n=0;for(int i=0;i<4;i++)*n=(*n<<8)|(unsigned char)s[i];return 1;}
static int onoff(const char *s){return !strcmp(s,"on")?1:!strcmp(s,"off")?0:-1;}
static int attr(int argc,char **av){
 int i=1,lock=-1,invisible=-1,has_type=0,has_creator=0;unsigned long type=0,creator=0;
 for(;i<argc&&av[i][0]=='-';i++){
  if(!strcmp(av[i],"--")){i++;break;}
  if(i+1>=argc)return 2;
  if(!strcmp(av[i],"--type")){has_type=code(av[++i],&type);if(!has_type)return 2;}
  else if(!strcmp(av[i],"--creator")){has_creator=code(av[++i],&creator);if(!has_creator)return 2;}
  else if(!strcmp(av[i],"--locked")){lock=onoff(av[++i]);if(lock<0)return 2;}
  else if(!strcmp(av[i],"--invisible")){invisible=onoff(av[++i]);if(invisible<0)return 2;}
  else return 2;
 }
 if(i==argc)return 2;int status=0;
 for(;i<argc;i++){
  char native[PATH_CAP],canonical[PATH_CAP],a[5],b[5],text[160];CInfoPBRec pb;
  int e=checked(av[i],native,canonical,0);if(!e)e=info(native,&pb);
  if(!e&&(has_type||has_creator||invisible>=0||lock>=0)){
   if(isdir(&pb)&&(has_type||has_creator||lock>=0))e=paramErr;
   if(!e){
    if(has_type)pb.hFileInfo.ioFlFndrInfo.fdType=type;
    if(has_creator)pb.hFileInfo.ioFlFndrInfo.fdCreator=creator;
    if(invisible>=0){unsigned short flags=isdir(&pb)?pb.dirInfo.ioDrUsrWds.frFlags:pb.hFileInfo.ioFlFndrInfo.fdFlags;flags=invisible?flags|0x4000:flags&~0x4000;if(isdir(&pb))pb.dirInfo.ioDrUsrWds.frFlags=flags;else pb.hFileInfo.ioFlFndrInfo.fdFlags=flags;}
    /* Unlock first only when explicitly requested. Never clear other flags. */
    if(lock==0)e=cli_native_lock(native,0);
    if(!e&&(has_type||has_creator||invisible>=0))e=cli_native_set(native,&pb);
    if(!e&&lock==1)e=cli_native_lock(native,1);
   }
  }else if(!e){
   say(av[i]);say(": ");
   if(isdir(&pb))snprintf(text,sizeof text,"directory flags=%04x invisible=%s\n",pb.dirInfo.ioDrUsrWds.frFlags,pb.dirInfo.ioDrUsrWds.frFlags&0x4000?"on":"off");
   else{four(pb.hFileInfo.ioFlFndrInfo.fdType,a);four(pb.hFileInfo.ioFlFndrInfo.fdCreator,b);snprintf(text,sizeof text,"type='%s' creator='%s' flags=%04x locked=%s invisible=%s\n",a,b,pb.hFileInfo.ioFlFndrInfo.fdFlags,pb.hFileInfo.ioFlAttrib&1?"on":"off",pb.hFileInfo.ioFlFndrInfo.fdFlags&0x4000?"on":"off");}say(text);
  }
  if(e)status=fail("hfsattr",e);if(app_yield())return 130;
 }return status;
}
static int mkdir_one(const char *path){char native[PATH_CAP],canonical[PATH_CAP],part[PATH_CAP];int e;
 if(!parents){e=checked(path,native,canonical,1);return e?e:cli_native_mkdir(native);}
 if(strlen(path)>=sizeof part)return bdNamErr;strcpy(part,path);size_t len=strlen(part);
 for(size_t i=1;i<=len;i++)if(part[i]=='/'||!part[i]){char c=part[i];part[i]=0;CInfoPBRec pb;e=resolve(part,native);if(!e)e=info(native,&pb);
  if(e==fnfErr||e==dirNFErr){e=checked(part,native,canonical,1);if(!e)e=cli_native_mkdir(native);}else if(!e&&!isdir(&pb))e=dirNFErr;
  part[i]=c;if(e)return e;if(app_yield())return 130;}return 0;}
static int remove_one(const char *path,int depth,int onlydir){
 char native[PATH_CAP],canonical[PATH_CAP],leaf[32],encoded[100],child[PATH_CAP];CInfoPBRec pb;int e=checked(path,native,canonical,0);if(!e)e=info(native,&pb);if((e==fnfErr||e==dirNFErr)&&force)return 0;if(e)return e;
 if(isdir(&pb)){if(!onlydir&&!recursive)return paramErr;if(recursive&&!onlydir){if(depth>=8)return paramErr;for(;;){e=cli_native_nth(native,1,leaf,&pb);if(e==fnfErr)break;if(e)return e;e=encode_name(leaf,encoded,sizeof encoded);if(!e)e=join_path(canonical,encoded,child);if(!e)e=remove_one(child,depth+1,0);if(e)return e;if(app_yield())return 130;}}}else if(onlydir)return dirNFErr;
 return cli_native_delete(native);
}
static int temporary_sibling(const char *destination,char *out){strcpy(out,destination);char *leaf=strrchr(out,':');if(!leaf)return bdNamErr;size_t prefix=leaf+1-out;if(prefix+30>255)return bdNamErr;for(int i=0;i<32;i++){snprintf(out+prefix,PATH_CAP-prefix,"Clv tmp %08lx %04lx",cli_ticks(),++sequence&65535);int e=cli_native_create(out,0,0);if(!e)return 0;if(e!=dupFNErr)return e;}return dupFNErr;}
static int copy_fork(const char *src,const char *dst,int resource){short in=-1,out=-1;char buf[4096];int e=cli_native_open(src,resource,0,&in);if(!e)e=cli_native_open(dst,resource,1,&out);while(!e){long n=sizeof buf;e=cli_native_read(in,buf,&n);if(e&&e!=eofErr)break;int eof=e==eofErr;e=0;if(n){long wrote=n;e=cli_native_write(out,buf,&wrote);if(!e&&wrote!=n)e=ioErr;}if(!e&&app_yield())e=130;if(e||eof||!n)break;}if(in>=0){int c=cli_native_close(in);if(!e)e=c;}if(out>=0){int c=cli_native_close(out);if(!e)e=c;}return e;}
static int copy_file(const char *src,const char *dst,CInfoPBRec *source){
 CInfoPBRec target,meta;char temp[PATH_CAP],backup[PATH_CAP];int exists=0,e=info(dst,&target);if(!e){exists=1;if(isdir(&target))return paramErr;if(strchr(src,':')-src==strchr(dst,':')-dst&&!strncasecmp(src,dst,strchr(src,':')-src)&&target.hFileInfo.ioDirID==source->hFileInfo.ioDirID)return paramErr;if(target.hFileInfo.ioFlAttrib&1)return fLckdErr;}else if(e!=fnfErr)return e;
 e=temporary_sibling(dst,temp);if(e)return e;e=copy_fork(src,temp,0);if(!e)e=copy_fork(src,temp,1);
 if(!e)e=info(temp,&meta);if(!e){meta.hFileInfo.ioFlFndrInfo=source->hFileInfo.ioFlFndrInfo;/* New file has no Finder position or desktop placement. */meta.hFileInfo.ioFlFndrInfo.fdFlags&=~0x0100;if(preserve){meta.hFileInfo.ioFlCrDat=source->hFileInfo.ioFlCrDat;meta.hFileInfo.ioFlMdDat=source->hFileInfo.ioFlMdDat;}e=cli_native_set(temp,&meta);}
 if(e){cli_native_delete(temp);return e;}
 if(exists){e=temporary_sibling(dst,backup);if(!e)e=cli_native_delete(backup);if(!e)e=cli_native_rename(dst,backup);if(e){cli_native_delete(temp);return e;}}
 e=cli_native_rename(temp,dst);if(e){if(exists){int restore=cli_native_rename(backup,dst);if(restore){errsay("copy recovery file: native:");errsay(backup);errsay("\n");}}cli_native_delete(temp);return e;}
 if(exists)e=cli_native_delete(backup);
 if(!e&&preserve&&(source->hFileInfo.ioFlAttrib&1))e=cli_native_lock(dst,1);return e;
}
static int copy_one(const char *source,const char *destination,int depth){
 char src[PATH_CAP],dst[PATH_CAP],sc[PATH_CAP],dc[PATH_CAP],leaf[32],enc[100],childsrc[PATH_CAP],childdst[PATH_CAP];CInfoPBRec pb,target;int e=resolve_ex(source,src,sc,0);if(!e)e=info(src,&pb);if(!e)e=checked(destination,dst,dc,0);if(e)return e;
 if(same_name(sc,dc))return paramErr;
 if(!isdir(&pb))return copy_file(src,dst,&pb);
 if(!recursive||depth>=8)return paramErr;size_t n=strlen(src);if(!strncasecmp(src,dst,n)&&(dst[n]==':'||!dst[n]))return paramErr;
 e=info(dst,&target);if(e==fnfErr){e=cli_native_mkdir(dst);}else if(!e&&!isdir(&target))e=paramErr;if(e)return e;
 for(int i=1;;i++){e=cli_native_nth(src,i,leaf,&target);if(e==fnfErr)break;if(e)return e;e=encode_name(leaf,enc,sizeof enc);if(!e)e=join_path(sc,enc,childsrc);if(!e)e=join_path(dc,enc,childdst);if(!e)e=copy_one(childsrc,childdst,depth+1);if(e)return e;if(app_yield())return 130;}
 return 0;
}
static int transfer(int argc,char **av,int start,int moving){
 if(argc-start<2)return 2;int status=0;char target[PATH_CAP],native[PATH_CAP],dc[PATH_CAP];CInfoPBRec pb;int targetdir=!resolve(av[argc-1],native)&&!info(native,&pb)&&isdir(&pb);if(argc-start>2&&!targetdir)return 2;
 for(int i=start;i<argc-1;i++){
  char src[PATH_CAP],sc[PATH_CAP],leaf[PATH_CAP];int e=checked(av[i],src,sc,0);if(e){status=fail(av[0],e);continue;}
  strcpy(target,av[argc-1]);if(targetdir){const char *base=strrchr(sc,'/');strcpy(leaf,base?base+1:sc);e=join_path(av[argc-1],leaf,target);}if(!e)e=checked(target,native,dc,0);
  if(!e&&same_name(sc,dc))e=paramErr;
  if(!e&&moving){CInfoPBRec dest;int absent=info(native,&dest)==fnfErr;if(absent){e=cli_native_rename(src,native);if(e==paramErr)e=cli_native_move(src,native);}else e=paramErr;
   if(e){e=info(src,&pb);if(!e&&isdir(&pb)){errsay("mv: directory moves require the same parent or unchanged leaf name\n");e=paramErr;}else if(!e){int saved=preserve;preserve=1;e=copy_one(av[i],target,0);preserve=saved;if(!e)e=cli_native_delete(src);}}
  }else if(!e)e=copy_one(av[i],target,0);
  if(e==130)return 130;if(e)status=fail(av[0],e);if(app_yield())return 130;
 }return status;
}
static int symlink_command(int argc,char **av){char native[PATH_CAP],canonical[PATH_CAP];if(argc!=4||strcmp(av[1],"-s")||!*av[2]||strlen(av[2])>=PATH_CAP)return 2;int e=checked(av[3],native,canonical,0);if(e)return fail("ln",e);e=cli_native_create(native,'S6LK','S6UX');if(e)return fail("ln",e);short ref;e=cli_native_open(native,0,1,&ref);if(!e){long n=5;e=cli_native_write(ref,"S6L1\n",&n);if(!e&&n!=5)e=ioErr;if(!e){n=strlen(av[2]);e=cli_native_write(ref,av[2],&n);if(!e&&n!=(long)strlen(av[2]))e=ioErr;}int c=cli_native_close(ref);if(!e)e=c;}if(e)cli_native_delete(native);return e?fail("ln",e):0;}
static void usage(const char *name){
 if(!strcmp(name,"hfsattr"))say("hfsattr [--type CODE] [--creator CODE] [--locked on|off] [--invisible on|off] [--] file ...\nNo options displays metadata. Codes are exactly four bytes; .... displays unset bytes.\nDirectories support visibility only. No Unix permissions are invented.\n");
 else if(!strcmp(name,"cp"))say("cp [-Rp] [--] source ... destination\nCopy data and resource forks and Finder type/creator. -R copies folders (8 levels).\n-p also preserves file creation/modification dates and locked state.\nSymlinks are copied as links. Directory metadata preservation is not implemented.\nExisting files are replaced only after both forks have copied successfully.\n");
 else if(!strcmp(name,"mv"))say("mv [--] source ... destination\nRename/move on HFS; regular files can cross volumes using fork-preserving copy.\nDirectory moves require the same parent or an unchanged leaf name.\n");
 else if(!strcmp(name,"touch"))say("touch [-c] [--] file ...\nUpdate modification time, or create an empty file with unset type/creator.\n-c does not create missing files. HFS has no access-time field; -a/-t are unsupported.\n");
 else if(!strcmp(name,"mkdir"))say("mkdir [-p] [--] directory ...\n-p creates missing parents and accepts existing directories.\n");
 else if(!strcmp(name,"rm"))say("rm [-frR] [--] file ...\n-r/-R removes folders recursively (8 levels); -f ignores absent files.\nLinks themselves are removed. Finder locks are respected even with -f.\n");
 else if(!strcmp(name,"rmdir"))say("rmdir [--] directory ...\nRemove empty directories only.\n");
 else if(!strcmp(name,"ln"))say("ln -s target link\nCreate a bounded Clarice symbolic link. Hard links and -f are unsupported.\n");
 else say("readlink [--] link ...\nPrint stored symbolic-link targets without resolving them.\n");
 say("Status: 0 success, 1 file error, 2 usage, 130 interrupted.\n");
}
int clarice_main(int argc,char **av){const char *name=strrchr(av[0],'/');name=name?name+1:av[0];if(argc==2&&!strcmp(av[1],"--help")){usage(name);return 0;}
 if(!strcmp(name,"hfsattr")){int rc=attr(argc,av);if(rc==2)usage(name);return rc;}
 if(!strcmp(name,"ln")){int rc=symlink_command(argc,av);if(rc==2)usage(name);return rc;}
 int i=1;for(;i<argc&&av[i][0]=='-'&&av[i][1];i++){if(!strcmp(av[i],"--")){i++;break;}for(int j=1;av[i][j];j++){char c=av[i][j];if((!strcmp(name,"cp")||!strcmp(name,"rm"))&&(c=='R'||c=='r'))recursive=1;else if(!strcmp(name,"cp")&&c=='p')preserve=1;else if(!strcmp(name,"rm")&&c=='f')force=1;else if(!strcmp(name,"mkdir")&&c=='p')parents=1;else if(!strcmp(name,"touch")&&c=='c')nocreate=1;else{usage(name);return 2;}}}
 if(!strcmp(name,"cp")||!strcmp(name,"mv")){int rc=transfer(argc,av,i,!strcmp(name,"mv"));if(rc==2)usage(name);return rc;}
 if(i==argc){if(!strcmp(name,"rm")&&force)return 0;usage(name);return 2;}int status=0;
 for(;i<argc;i++){char native[PATH_CAP],canonical[PATH_CAP],target[PATH_CAP];CInfoPBRec pb;int e=0;
  if(!strcmp(name,"mkdir"))e=mkdir_one(av[i]);
  else if(!strcmp(name,"rm")||!strcmp(name,"rmdir"))e=remove_one(av[i],0,!strcmp(name,"rmdir"));
  else if(!strcmp(name,"readlink")){e=resolve_ex(av[i],native,canonical,0);if(!e)e=link_target(native,target);if(!e){say(target);say("\n");}}
  else if(!strcmp(name,"touch")){e=checked(av[i],native,canonical,1);if(!e){e=info(native,&pb);if(e==fnfErr){if(nocreate)e=0;else e=cli_native_create(native,0,0);}else if(!e){if(isdir(&pb))pb.dirInfo.ioDrMdDat=cli_mac_time();else pb.hFileInfo.ioFlMdDat=cli_mac_time();e=cli_native_set(native,&pb);}}}
  else return 127;
  if(e==130)return 130;if(e)status=fail(name,e);if(app_yield())return 130;
 }return output_error?1:status;
}
