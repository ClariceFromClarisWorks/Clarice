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
 if(!*path||strlen(path)>=sizeof part)return bdNamErr;strcpy(part,path);size_t len=strlen(part);
 for(size_t i=1;i<=len;i++)if(part[i]=='/'||!part[i]){char c=part[i];part[i]=0;CInfoPBRec pb;e=resolve(part,native);if(!e)e=info(native,&pb);
  if(e==fnfErr||e==dirNFErr){e=checked(part,native,canonical,1);if(!e)e=cli_native_mkdir(native);}else if(!e&&!isdir(&pb))e=dirNFErr;
  part[i]=c;if(e)return e;if(app_yield())return 130;}return 0;}
/* Explicit DFS frames keep stack use constant; only active directories allocate. */
typedef struct RemoveFrame {struct RemoveFrame *parent;char native[PATH_CAP],path[PATH_CAP];} RemoveFrame;
static int remove_one(const char *path,int depth,int onlydir){
 (void)depth;RemoveFrame *top=NULL;char pending[PATH_CAP],native[PATH_CAP],canonical[PATH_CAP],leaf[32],encoded[100];CInfoPBRec pb;int e=0;
 /* Reject dot operands before resolution erases their spelling or traversal
    deletes contents. Ignore trailing separators, as Unix rm does. */
 size_t end=strlen(path),start;if(end>=sizeof pending)return bdNamErr;
 while(end&&path[end-1]=='/')end--;start=end;while(start&&path[start-1]!='/')start--;
 if(end-start==1&&path[start]=='.')return paramErr;
 if(end-start==2&&path[start]=='.'&&path[start+1]=='.')return paramErr;
 strcpy(pending,path);
 for(;;){
  if(app_yield()){e=130;break;}
  e=checked(pending,native,canonical,0);if(!e)e=info(native,&pb);
  if((e==fnfErr||e==dirNFErr)&&force)e=0;
  else if(!e){
   if(isdir(&pb)&&recursive&&!onlydir){
    RemoveFrame *f=cli_alloc(sizeof *f);if(!f){e=memFullErr;break;}
    f->parent=top;strcpy(f->native,native);strcpy(f->path,canonical);top=f;
   }else if(isdir(&pb)&&!onlydir)e=paramErr;
   else if(!isdir(&pb)&&onlydir)e=dirNFErr;
   else e=cli_native_delete(native);
  }
  if(e)break;
  while(top){
   e=cli_native_nth(top->native,1,leaf,&pb);
   if(!e){e=encode_name(leaf,encoded,sizeof encoded);if(!e)e=join_path(top->path,encoded,pending);break;}
   if(e!=fnfErr)break;
   e=cli_native_delete(top->native);if(e)break;
   RemoveFrame *f=top;top=f->parent;cli_free(f);
   if(app_yield()){e=130;break;}
  }
  if(e||!top)break;
 }
 while(top){RemoveFrame *f=top;top=f->parent;cli_free(f);}return e;
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
typedef struct CopyFrame {struct CopyFrame *parent;char native[PATH_CAP],source[PATH_CAP],destination[PATH_CAP];int index;} CopyFrame;
static int copy_one(const char *source,const char *destination,int depth){
 (void)depth;CopyFrame *top=NULL;char pending_src[PATH_CAP],pending_dst[PATH_CAP],src[PATH_CAP],dst[PATH_CAP],sc[PATH_CAP],dc[PATH_CAP],leaf[32],enc[100];CInfoPBRec pb,target;int e=0;
 if(strlen(source)>=PATH_CAP||strlen(destination)>=PATH_CAP)return bdNamErr;
 strcpy(pending_src,source);strcpy(pending_dst,destination);
 for(;;){
  if(app_yield()){e=130;break;}
  e=resolve_ex(pending_src,src,sc,0);if(!e)e=info(src,&pb);if(!e)e=checked(pending_dst,dst,dc,0);if(e)break;
  if(same_name(sc,dc)){e=paramErr;break;}
  if(!isdir(&pb))e=copy_file(src,dst,&pb);
  else{
   size_t n=strlen(src);if(!recursive||(!strncasecmp(src,dst,n)&&(dst[n]==':'||!dst[n]))){e=paramErr;break;}
   CopyFrame *f=cli_alloc(sizeof *f);if(!f){e=memFullErr;break;}
   f->parent=top;f->index=1;strcpy(f->native,src);strcpy(f->source,sc);strcpy(f->destination,dc);top=f;
   e=info(dst,&target);if(e==fnfErr)e=cli_native_mkdir(dst);else if(!e&&!isdir(&target))e=paramErr;
  }
  if(e)break;
  while(top){
   if(top->index>32767){e=paramErr;break;}
   e=cli_native_nth(top->native,top->index++,leaf,&target);
   if(!e){e=encode_name(leaf,enc,sizeof enc);if(!e)e=join_path(top->source,enc,pending_src);if(!e)e=join_path(top->destination,enc,pending_dst);break;}
   if(e!=fnfErr)break;
   CopyFrame *f=top;top=f->parent;cli_free(f);e=0;
   if(app_yield()){e=130;break;}
  }
  if(e||!top)break;
 }
 while(top){CopyFrame *f=top;top=f->parent;cli_free(f);}return e;
}
static int transfer(int argc,char **av,int start,int moving){
 if(argc-start<2)return 2;int status=0;char target[PATH_CAP],native[PATH_CAP],dc[PATH_CAP];CInfoPBRec pb;int targetdir=!resolve(av[argc-1],native)&&!info(native,&pb)&&isdir(&pb);if(argc-start>2&&!targetdir)return 2;
 for(int i=start;i<argc-1;i++){
  char src[PATH_CAP],sc[PATH_CAP],leaf[PATH_CAP];int e=checked(av[i],src,sc,0);if(e){status=fail(av[0],e);continue;}
  strcpy(target,av[argc-1]);if(targetdir){const char *base=strrchr(sc,'/');strcpy(leaf,base?base+1:sc);e=join_path(av[argc-1],leaf,target);}if(!e)e=checked(target,native,dc,0);
  if(!e&&same_name(sc,dc))e=paramErr;
  if(!e&&moving){CInfoPBRec dest;int target_error=info(native,&dest);
   e=info(src,&pb);
   /* A known undeletable source must not commit a replacement first. */
   if(!e&&(pb.hFileInfo.ioFlAttrib&1))e=fLckdErr;
   if(!e&&target_error&&target_error!=fnfErr)e=target_error;
   if(!e){
    size_t volume_len=strchr(src,':')-src;
    int cross_volume=(size_t)(strchr(native,':')-native)!=volume_len||strncasecmp(src,native,volume_len);
    if(target_error==fnfErr&&!cross_volume){e=cli_native_rename(src,native);if(e==paramErr)e=cli_native_move(src,native);}
    else e=paramErr;
    /* Only an unsupported native move needs copy/delete. Preserve genuine
       permission, lock, I/O and catalog errors without touching the target. */
    if(e==paramErr){if(isdir(&pb)){errsay("mv: directory moves require the same parent or unchanged leaf name\n");}
     else{int saved=preserve;preserve=1;e=copy_one(av[i],target,0);preserve=saved;
      if(!e){e=cli_native_delete(src);if(e)errsay("mv: destination copied, but source could not be removed\n");}}
    }
   }
  }else if(!e)e=copy_one(av[i],target,0);
  if(e==130)return 130;if(e)status=fail(av[0],e);if(app_yield())return 130;
 }return status;
}
static int symlink_command(int argc,char **av){char native[PATH_CAP],canonical[PATH_CAP];if(argc!=4||strcmp(av[1],"-s")||!*av[2]||strlen(av[2])>=PATH_CAP)return 2;int e=checked(av[3],native,canonical,0);if(e)return fail("ln",e);e=cli_native_create(native,'S6LK','S6UX');if(e)return fail("ln",e);short ref;e=cli_native_open(native,0,1,&ref);if(!e){long n=5;e=cli_native_write(ref,"S6L1\n",&n);if(!e&&n!=5)e=ioErr;if(!e){n=strlen(av[2]);e=cli_native_write(ref,av[2],&n);if(!e&&n!=(long)strlen(av[2]))e=ioErr;}int c=cli_native_close(ref);if(!e)e=c;}if(e)cli_native_delete(native);return e?fail("ln",e):0;}
static void usage_text(int fd,const char *s){cli_write(fd,s,strlen(s));}
static void usage(const char *name,int fd){
 if(!strcmp(name,"hfsattr"))usage_text(fd,"hfsattr [--type CODE] [--creator CODE] [--locked on|off] [--invisible\non|off] [--] file ...\nNo options displays metadata. Codes are exactly four bytes; ....\ndisplays unset bytes.\nDirectories support visibility only. No Unix permissions are invented.\n");
 else if(!strcmp(name,"cp"))usage_text(fd,"cp [-Rp] [--] source ... destination\nCopy data and resource forks and Finder type/creator. -R copies folders\nusing available traversal memory.\n-p also preserves file creation/modification dates and locked state.\nSymlinks are copied as links. Directory metadata preservation is not\nimplemented.\nExisting files are replaced only after both forks have copied\nsuccessfully.\n");
 else if(!strcmp(name,"mv"))usage_text(fd,"mv [--] source ... destination\nRename/move on HFS; regular files can cross volumes using\nfork-preserving copy.\nDirectory moves require the same parent or an unchanged leaf name.\n");
 else if(!strcmp(name,"touch"))usage_text(fd,"touch [-c] [--] file ...\nUpdate modification time, or create an empty file with unset\ntype/creator.\n-c does not create missing files. HFS has no access-time field; -a/-t\nare unsupported.\n");
 else if(!strcmp(name,"mkdir"))usage_text(fd,"mkdir [-p] [--] directory ...\n-p creates missing parents and accepts existing directories.\n");
 else if(!strcmp(name,"rm"))usage_text(fd,"rm [-frR] [--] file ...\n-r/-R removes folders recursively using available traversal memory; -f\nignores absent files.\nLinks themselves are removed. Finder locks are respected even with -f.\n");
 else if(!strcmp(name,"rmdir"))usage_text(fd,"rmdir [--] directory ...\nRemove empty directories only.\n");
 else if(!strcmp(name,"ln"))usage_text(fd,"ln -s target link\nCreate a bounded Clarice symbolic link. Hard links and -f are\nunsupported.\n");
 else usage_text(fd,"readlink [--] link ...\nPrint stored symbolic-link targets without resolving them.\n");
 usage_text(fd,"Status: 0 success, 1 file error, 2 usage, 130 interrupted.\n");
}
int clarice_main(int argc,char **av){const char *name=strrchr(av[0],'/');name=name?name+1:av[0];if(argc==2&&!strcmp(av[1],"--help")){usage(name,1);return 0;}
 if(!strcmp(name,"hfsattr")){int rc=attr(argc,av);if(rc==2)usage(name,2);return rc;}
 if(!strcmp(name,"ln")){int rc=symlink_command(argc,av);if(rc==2)usage(name,2);return rc;}
 int i=1;for(;i<argc&&av[i][0]=='-'&&av[i][1];i++){if(!strcmp(av[i],"--")){i++;break;}for(int j=1;av[i][j];j++){char c=av[i][j];if((!strcmp(name,"cp")||!strcmp(name,"rm"))&&(c=='R'||c=='r'))recursive=1;else if(!strcmp(name,"cp")&&c=='p')preserve=1;else if(!strcmp(name,"rm")&&c=='f')force=1;else if(!strcmp(name,"mkdir")&&c=='p')parents=1;else if(!strcmp(name,"touch")&&c=='c')nocreate=1;else{usage(name,2);return 2;}}}
 if(!strcmp(name,"cp")||!strcmp(name,"mv")){int rc=transfer(argc,av,i,!strcmp(name,"mv"));if(rc==2)usage(name,2);return rc;}
 if(i==argc){if(!strcmp(name,"rm")&&force)return 0;usage(name,2);return 2;}int status=0;
 for(;i<argc;i++){char native[PATH_CAP],canonical[PATH_CAP],target[PATH_CAP];CInfoPBRec pb;int e=0;
  if(!strcmp(name,"mkdir"))e=mkdir_one(av[i]);
  else if(!strcmp(name,"rm")||!strcmp(name,"rmdir"))e=remove_one(av[i],0,!strcmp(name,"rmdir"));
  else if(!strcmp(name,"readlink")){e=resolve_ex(av[i],native,canonical,0);if(!e)e=link_target(native,target);if(!e){say(target);say("\n");}}
  else if(!strcmp(name,"touch")){e=checked(av[i],native,canonical,1);if(!e){e=info(native,&pb);if(e==fnfErr){if(nocreate)e=0;else e=cli_native_create(native,0,0);}else if(!e){if(isdir(&pb))pb.dirInfo.ioDrMdDat=cli_mac_time();else pb.hFileInfo.ioFlMdDat=cli_mac_time();e=cli_native_set(native,&pb);}}}
  else return 127;
  if(e==130)return 130;if(e)status=fail(name,e);if(app_yield())return 130;
 }return output_error?1:status;
}
