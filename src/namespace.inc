/* Bounded Unix namespace backed by ordinary HFS folders and mounted volumes. */
static int info(const char *path,CInfoPBRec *pb);
static int same_name(const char *a,const char *b){while(*a&&*b)if(tolower((unsigned char)*a++)!=tolower((unsigned char)*b++))return 0;return !*a&&!*b;}
static int encode_name(const char *s,char *out,size_t cap){size_t n=0;while(*s){unsigned char c=*s++;if(n+4>=cap)return bdNamErr;if(c=='/'||c=='%'){out[n++]='%';out[n++]='2';out[n++]=c=='/'?'F':'5';}else out[n++]=c;}out[n]=0;return 0;}
static int decode_name(const char *s,size_t n,char *out){size_t i,j=0;for(i=0;i<n;i++){unsigned char c=s[i];if(c=='%'&&i+2<n&&isxdigit((unsigned char)s[i+1])&&isxdigit((unsigned char)s[i+2])){char h[3]={s[i+1],s[i+2],0};c=strtol(h,0,16);i+=2;}if(!c||c==':'||j>=31)return bdNamErr;out[j++]=c;}out[j]=0;return 0;}
static int volume_name(int index,char *name){HParamBlockRec pb;unsigned char n[256];memset(&pb,0,sizeof pb);pb.volumeParam.ioNamePtr=n;pb.volumeParam.ioVolIndex=index;int e=PBHGetVInfoSync(&pb);if(e)return e;memcpy(name,n+1,n[0]);name[n[0]]=0;return 0;}
/* Volume labels are never silently resolved to the wrong duplicate volume. */
static int volume_lookup(const char *name,short *ref){
 int hits=0,e;short found=0;
 for(int i=1;;i++){HParamBlockRec pb;unsigned char label[256];memset(&pb,0,sizeof pb);pb.volumeParam.ioNamePtr=label;pb.volumeParam.ioVolIndex=i;
  e=PBHGetVInfoSync(&pb);if(e)break;label[label[0]+1]=0;
  if(same_name(name,(char*)label+1)){hits++;found=pb.volumeParam.ioVRefNum;}
 }
 if(e!=nsvErr)return e;if(hits!=1)return hits?dupFNErr:nsvErr;
 if(!found)return nsvErr; /* Never return the default-volume selector. */
 if(ref)*ref=found;return 0;
}
static int volume_unique(const char *name){return volume_lookup(name,0);}
static int cli_native_volume(const char *path,short *ref){
 char volume[32];const char *end=strchr(path,':');
 if(!end||end==path||end-path>31)return bdNamErr;
 memcpy(volume,path,end-path);volume[end-path]=0;return volume_lookup(volume,ref);
}
static int native_path(const char *u,char *out){const char *p=u;char part[32];size_t len;
 if(!strncmp(p,"/mnt/",5)){p+=5;const char *end=strchr(p,'/');size_t n=end?(size_t)(end-p):strlen(p);int e=decode_name(p,n,part);if(e)return e;if((e=volume_unique(part)))return e;strcpy(out,part);strcat(out,":");p+=n;if(*p=='/')p++;}
 else {strcpy(out,root_path);if(*p=='/')p++;}
 len=strlen(out);while(*p){const char *end=strchr(p,'/');size_t n=end?(size_t)(end-p):strlen(p);int e=decode_name(p,n,part);if(e)return e;size_t k=strlen(part);if(len+k+1>255)return bdNamErr;memcpy(out+len,part,k);len+=k;p+=n;if(*p=='/'){out[len++]=':';p++;}out[len]=0;}return 0;
}
static int unix_from_native(const char *native,char *out){char copy[PATH_CAP],encoded[100];const char *p=native;size_t base=strlen(root_path);if(!strncmp(native,root_path,base)){strcpy(out,"/");p+=base;}else strcpy(out,"/mnt/");
 if(strlen(p)>=sizeof copy)return bdNamErr;strcpy(copy,p);char *part=copy;while(*part){char *colon=strchr(part,':');if(colon)*colon=0;if(encode_name(part,encoded,sizeof encoded)||strlen(out)+strlen(encoded)+2>=PATH_CAP)return bdNamErr;strcat(out,encoded);if(!colon)break;part=colon+1;if(*part)strcat(out,"/");}return 0;
}
static int is_link(const CInfoPBRec *pb){return !(pb->hFileInfo.ioFlAttrib&16)&&pb->hFileInfo.ioFlFndrInfo.fdType=='S6LK'&&pb->hFileInfo.ioFlFndrInfo.fdCreator=='S6UX';}
static int link_target(const char *native,char *target){unsigned char p[256];char b[PATH_CAP+5];short ref;long n=sizeof b;CInfoPBRec pb;int e=info(native,&pb);if(e)return e;if(!is_link(&pb))return paramErr;pstr(p,native);e=HOpen(0,0,p,fsRdPerm,&ref);if(e)return e;e=FSRead(ref,&n,b);FSClose(ref);if((e&&e!=eofErr)||n<=5||n>PATH_CAP+4||memcmp(b,"S6L1\n",5)||memchr(b+5,0,n-5))return paramErr;memcpy(target,b+5,n-5);target[n-5]=0;return 0;}
static void parent_path(char *s){char *p=strrchr(s,'/');if(p==s)s[1]=0;else if(p)*p=0;}
/* Resolve one component at a time: a link/.. applies to the target's parent.
   No recursion, fixed buffers, at most 16 traversals per operation. */
static int resolve_ex(const char *s,char *native,char *canonical,int follow){char pending[PATH_CAP],done[PATH_CAP]="/",part[100],target[PATH_CAP];int links=0;
 if(!strncmp(s,"native:",7)){int e=unix_from_native(s+7,pending);if(e)return e;}
 else if(*s=='~'&&(s[1]=='/'||!s[1])){if(strlen(s)>=PATH_CAP)return bdNamErr;strcpy(pending,s+1);if(!*pending)strcpy(pending,"/");}
 else if(*s=='/'){if(strlen(s)>=PATH_CAP)return bdNamErr;strcpy(pending,s);}
 else {if(strlen(unix_cwd)+strlen(s)+2>=PATH_CAP)return bdNamErr;strcpy(pending,unix_cwd);if(strcmp(pending,"/"))strcat(pending,"/");strcat(pending,s);}
 while(*pending){while(*pending=='/')memmove(pending,pending+1,strlen(pending));if(!*pending)break;char *slash=strchr(pending,'/');size_t n=slash?(size_t)(slash-pending):strlen(pending);if(n>=sizeof part)return bdNamErr;memcpy(part,pending,n);part[n]=0;memmove(pending,pending+n,strlen(pending+n)+1);
  if(!strcmp(part,"."))continue;if(!strcmp(part,"..")){parent_path(done);continue;}
  size_t old=strlen(done);if(old+strlen(part)+2>=PATH_CAP)return bdNamErr;if(old>1)strcat(done,"/");strcat(done,part);
  int e=native_path(done,native);if(e)return e;CInfoPBRec pb;e=info(native,&pb);
  if(e){if(e==fnfErr&&!*pending)break;return e;}
  if(is_link(&pb)&&(*pending||follow)){if(++links>16)return paramErr;e=link_target(native,target);if(e)return e;if(strlen(target)+strlen(pending)>=PATH_CAP)return bdNamErr;strcat(target,pending);strcpy(pending,target);done[old]=0;if(*target=='/')strcpy(done,"/");continue;}
  if(*pending&&!(pb.hFileInfo.ioFlAttrib&16))return dirNFErr;
 }
 int e=native_path(done,native);if(!e&&canonical)strcpy(canonical,done);return e;
}
static int resolve(const char *s,char *out){return resolve_ex(s,out,0,1);}
static int protected_path(const char *canonical){if(!strcmp(canonical,"/")||!strcmp(canonical,"/mnt"))return 1;if(!strncmp(canonical,"/mnt/",5)&&!strchr(canonical+5,'/'))return 1;return 0;}
static int create_dir(const unsigned char *name,long *id){HParamBlockRec pb;memset(&pb,0,sizeof pb);pb.fileParam.ioNamePtr=(unsigned char*)name;int e=PBDirCreateSync(&pb);if(!e)*id=pb.fileParam.ioDirID;return e;}
void shell_prompt(char *out,size_t cap){const char *dir=strrchr(unix_cwd,'/');dir=dir&&dir[1]?dir+1:"/";const char *host=env_get("HOSTNAME");char safe[33];int n=0;while(*host&&n<32){unsigned char c=*host++;if(isalnum(c)||c=='-'||c=='_'||c=='.')safe[n++]=c;}safe[n]=0;if(n)snprintf(out,cap,"%s %s > ",safe,dir);else snprintf(out,cap,"%s > ",dir);}
/* First-run installer. Locate the running System's actual directory by WD ID,
   never by assuming its folder name or the application's launch volume. */
static int namespace_install(void){SysEnvRec sys;WDPBRec wd;HParamBlockRec vp;CInfoPBRec cat;unsigned char name[256];char tail[PATH_CAP]="",part[PATH_CAP];int e;long id;
 e=SysEnvirons(1,&sys);if(e)return e;memset(&wd,0,sizeof wd);wd.ioVRefNum=sys.sysVRefNum;e=PBGetWDInfoSync(&wd);if(e)return e;
 memset(&vp,0,sizeof vp);vp.volumeParam.ioVRefNum=wd.ioWDVRefNum;vp.volumeParam.ioNamePtr=name;e=PBHGetVInfoSync(&vp);if(e)return e;memcpy(root_path,name+1,name[0]);root_path[name[0]]=':';root_path[name[0]+1]=0;
 id=wd.ioWDDirID;if(id<=2)id=vp.volumeParam.ioVFndrInfo[0];if(id<=2)return dirNFErr;
 for(int depth=0;id>2&&depth<32;depth++){memset(&cat,0,sizeof cat);cat.dirInfo.ioNamePtr=name;cat.dirInfo.ioVRefNum=wd.ioWDVRefNum;cat.dirInfo.ioDrDirID=id;cat.dirInfo.ioFDirIndex=-1;e=PBGetCatInfoSync(&cat);if(e)return e;if(name[0]+strlen(tail)+2>=sizeof part)return bdNamErr;memcpy(part,name+1,name[0]);part[name[0]]=':';strcpy(part+name[0]+1,tail);strcpy(tail,part);id=cat.dirInfo.ioDrParID;}
 if(id!=2||strlen(root_path)+strlen(tail)+32>255)return bdNamErr;strcat(root_path,tail);strcat(root_path,"Clarice Data:");
 static const char *dirs[]={"","bin","etc","etc:clarice","tmp","dev","mnt","usr","usr:bin",0};for(int i=0;dirs[i];i++){snprintf(part,sizeof part,"%s%s",root_path,dirs[i]);size_t n=strlen(part);if(part[n-1]==':')part[n-1]=0;pstr(name,part);e=create_dir(name,&id);if(e==dupFNErr){e=info(part,&cat);if(!e&&!(cat.hFileInfo.ioFlAttrib&16))e=dirNFErr;}if(e)return e;}
 snprintf(part,sizeof part,"%.240setc:profile",root_path);pstr(name,part);e=HCreate(0,0,name,'S6UX','TEXT');if(e==dupFNErr)return 0;if(e)return e;short ref;e=HOpen(0,0,name,fsWrPerm,&ref);if(!e){static const char profile[]="# Clarice startup profile; sourced once at launch.\nexport PATH=/bin:/usr/bin\n# Optional local hostname (no networking required):\n# export HOSTNAME=macplus\n";long n=sizeof profile-1;e=FSWrite(ref,&n,profile);if(!e&&n!=sizeof profile-1)e=ioErr;FSClose(ref);}if(e)HDelete(0,0,name);FlushVol(0,0);return e;
}

/* Rename within a catalog directory: System 6 rejects a full source path
   for this operation on some HFS drivers. Resolve volume and parent first. */
int shell_rename(const unsigned char *source,const unsigned char *newname){
 char path[256];unsigned char volume[256],leaf[256];CInfoPBRec cat;HParamBlockRec pb;
 memcpy(path,source+1,source[0]);path[source[0]]=0;char *colon=strchr(path,':'),*last=strrchr(path,':');if(!colon||!last||!last[1])return bdNamErr;
 int e=info(path,&cat);if(e)return e;int len=colon-path;volume[0]=len;memcpy(volume+1,path,len);pstr(leaf,last+1);
 memset(&pb,0,sizeof pb);pb.volumeParam.ioNamePtr=volume;e=PBHGetVInfoSync(&pb);if(e)return e;
 short vr=pb.volumeParam.ioVRefNum;long parent=cat.hFileInfo.ioFlParID;
 memset(&pb,0,sizeof pb);pb.fileParam.ioVRefNum=vr;pb.fileParam.ioDirID=parent;pb.fileParam.ioNamePtr=leaf;pb.ioParam.ioMisc=(Ptr)newname;return PBHRenameSync(&pb);
}
void shell_temp_path(char *out,const char *file){snprintf(out,PATH_CAP,"%.220stmp:%.31s",root_path,file);}
/* Copy migration commits by rename, never overwrites an existing destination.
   Failure leaves the legacy file usable. Fixed 512-byte transfer buffer. */
static int migrate_private_file(char *out,const char *folder,const char *file,const char *legacy){
 char old[PATH_CAP],temp[PATH_CAP];unsigned char dest[256],src[256],tmp[256],base[256];CInfoPBRec pb;short in=-1,to=-1;long size=0;int e;
 if(strlen(root_path)+strlen(folder)+strlen(file)>255){out[0]=0;return bdNamErr;}strcpy(out,root_path);strcat(out,folder);strcat(out,file);
 e=info(out,&pb);if(!e)return 0;if(e!=fnfErr)return e;
 snprintf(old,sizeof old,"%s%s",root_path,legacy);e=info(old,&pb);if(e==fnfErr)return 0;if(e)goto fail;if((pb.hFileInfo.ioFlAttrib&16)||pb.hFileInfo.ioFlRLgLen){e=paramErr;goto fail;}
 pstr(src,old);pstr(dest,out);strcpy(temp,root_path);strcat(temp,folder);strcat(temp,"migrate.tmp");pstr(tmp,temp);pstr(base,file);
 e=HOpen(0,0,src,fsRdPerm,&in);if(e)goto fail;e=GetEOF(in,&size);if(e||size>132000){e=paramErr;goto fail;}
 e=HCreate(0,0,tmp,'S6UX','DATA');if(e)goto fail;
 e=HOpen(0,0,tmp,fsWrPerm,&to);if(!e){char buffer[512];while(size&&!e){long n=size>512?512:size,want=n;e=FSRead(in,&n,buffer);if(!e&&n!=want)e=ioErr;if(!e){e=FSWrite(to,&n,buffer);if(!e&&n!=want)e=ioErr;}size-=want;}}
 if(to>=0){int close=FSClose(to);to=-1;if(!e)e=close;}if(!e)e=FlushVol(0,0);
 if(!e)e=shell_rename(tmp,base);if(e)HDelete(0,0,tmp);
 if(in>=0){FSClose(in);in=-1;}if(!e){HDelete(0,0,src);FlushVol(0,0);return 0;}
 fail:if(in>=0)FSClose(in);strcpy(out,old);{char msg[100];snprintf(msg,sizeof msg,"State migration deferred (%d); original file retained.\n",e);term_puts(msg);}return e;
}

int shell_state_path(char *out,const char *file,const char *legacy){return migrate_private_file(out,"etc:clarice:",file,legacy);}
static void migrate_reports(void){
 static const char *old[]={"Cached Scroll Report","Native Scroll Report","Scroll Report","Phase1 Report","Drag Report"};
 static const char *names[]={"legacy-cached-scroll.txt","legacy-native-scroll.txt","legacy-scroll.txt","legacy-phase1.txt","legacy-drag.txt"};
 for(int i=0;i<5;i++){char path[PATH_CAP];migrate_private_file(path,"tmp:",names[i],old[i]);}
}
