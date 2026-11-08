/* Bounded Unix namespace backed by ordinary HFS folders and mounted volumes. */
static int info(const char *path,CInfoPBRec *pb);
static int same_name(const char *a,const char *b){while(*a&&*b)if(tolower((unsigned char)*a++)!=tolower((unsigned char)*b++))return 0;return !*a&&!*b;}
static int encode_name(const char *s,char *out,size_t cap){size_t n=0;while(*s){unsigned char c=*s++;if(n+4>=cap)return bdNamErr;if(c=='/'||c=='%'){out[n++]='%';out[n++]='2';out[n++]=c=='/'?'F':'5';}else out[n++]=c;}out[n]=0;return 0;}
static int decode_name(const char *s,size_t n,char *out){size_t i,j=0;for(i=0;i<n;i++){unsigned char c=s[i];if(c=='%'&&i+2<n&&isxdigit((unsigned char)s[i+1])&&isxdigit((unsigned char)s[i+2])){char h[3]={s[i+1],s[i+2],0};c=strtol(h,0,16);i+=2;}if(!c||c==':'||j>=31)return bdNamErr;out[j++]=c;}out[j]=0;return 0;}
static int volume_name(int index,char *name){HParamBlockRec pb;unsigned char n[256];memset(&pb,0,sizeof pb);pb.volumeParam.ioNamePtr=n;pb.volumeParam.ioVolIndex=index;int e=PBHGetVInfoSync(&pb);if(e)return e;memcpy(name,n+1,n[0]);name[n[0]]=0;return 0;}
/* Volume labels are never silently resolved to the wrong duplicate volume. */
static int volume_unique(const char *name){int i,hits=0,e;char label[32];for(i=1;!(e=volume_name(i,label));i++){if(same_name(name,label))hits++;}if(e!=nsvErr)return e;return hits==1?0:hits?dupFNErr:nsvErr;}
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
static int namespace_command(int argc,char **av){char native[PATH_CAP],canonical[PATH_CAP],target[PATH_CAP];unsigned char name[256];int e;
 if(!strcmp(av[0],"ln")){if(argc!=4||strcmp(av[1],"-s")){say("usage: ln -s target link (hard links unsupported)\n");return 2;}if(!*av[2]||strlen(av[2])>=PATH_CAP)return 2;e=resolve_ex(av[3],native,canonical,0);if(!e&&protected_path(canonical))e=paramErr;if(!e){short ref;pstr(name,native);e=HCreate(0,0,name,'S6UX','S6LK');if(!e){e=HOpen(0,0,name,fsWrPerm,&ref);if(!e){long n=5;e=FSWrite(ref,&n,"S6L1\n");if(!e&&n!=5)e=ioErr;if(!e){n=strlen(av[2]);e=FSWrite(ref,&n,av[2]);if(!e&&n!=(long)strlen(av[2]))e=ioErr;}FSClose(ref);}if(e)HDelete(0,0,name);}}}
 else if(!strcmp(av[0],"readlink")){if(argc!=2)return 2;e=resolve_ex(av[1],native,canonical,0);if(!e)e=link_target(native,target);if(!e){say(target);say("\n");}}
 else {if(argc!=2){say("usage: mkdir directory | rm file\n");return 2;}e=resolve_ex(av[1],native,canonical,0);if(!e&&protected_path(canonical))e=paramErr;if(!e){pstr(name,native);if(!strcmp(av[0],"mkdir")){long id;e=create_dir(name,&id);}else{CInfoPBRec pb;e=info(native,&pb);if(!e&&(pb.hFileInfo.ioFlAttrib&16))e=paramErr;if(!e)e=HDelete(0,0,name);}}}
 if(e){error(av[0],e);return 1;}return 0;
}
void shell_prompt(char *out,size_t cap){const char *dir=strrchr(unix_cwd,'/');dir=dir&&dir[1]?dir+1:"/";const char *host=env_get("HOSTNAME");char safe[33];int n=0;while(*host&&n<32){unsigned char c=*host++;if(isalnum(c)||c=='-'||c=='_'||c=='.')safe[n++]=c;}safe[n]=0;if(n)snprintf(out,cap,"%s %s > ",safe,dir);else snprintf(out,cap,"%s > ",dir);}
/* First-run installer. Locate the running System's actual directory by WD ID,
   never by assuming its folder name or the application's launch volume. */
static int namespace_install(void){SysEnvRec sys;WDPBRec wd;HParamBlockRec vp;CInfoPBRec cat;unsigned char name[256];char tail[PATH_CAP]="",part[PATH_CAP];int e;long id;
 e=SysEnvirons(1,&sys);if(e)return e;memset(&wd,0,sizeof wd);wd.ioVRefNum=sys.sysVRefNum;e=PBGetWDInfoSync(&wd);if(e)return e;
 memset(&vp,0,sizeof vp);vp.volumeParam.ioVRefNum=wd.ioWDVRefNum;vp.volumeParam.ioNamePtr=name;e=PBHGetVInfoSync(&vp);if(e)return e;memcpy(root_path,name+1,name[0]);root_path[name[0]]=':';root_path[name[0]+1]=0;
 id=wd.ioWDDirID;if(id<=2)id=vp.volumeParam.ioVFndrInfo[0];if(id<=2)return dirNFErr;
 for(int depth=0;id>2&&depth<32;depth++){memset(&cat,0,sizeof cat);cat.dirInfo.ioNamePtr=name;cat.dirInfo.ioVRefNum=wd.ioWDVRefNum;cat.dirInfo.ioDrDirID=id;cat.dirInfo.ioFDirIndex=-1;e=PBGetCatInfoSync(&cat);if(e)return e;if(name[0]+strlen(tail)+2>=sizeof part)return bdNamErr;memcpy(part,name+1,name[0]);part[name[0]]=':';strcpy(part+name[0]+1,tail);strcpy(tail,part);id=cat.dirInfo.ioDrParID;}
 if(id!=2||strlen(root_path)+strlen(tail)+32>255)return bdNamErr;strcat(root_path,tail);strcat(root_path,"Clarice Data:");
 static const char *dirs[]={"","bin","etc","tmp","dev","mnt","usr","usr:bin",0};for(int i=0;dirs[i];i++){snprintf(part,sizeof part,"%s%s",root_path,dirs[i]);size_t n=strlen(part);if(part[n-1]==':')part[n-1]=0;pstr(name,part);e=create_dir(name,&id);if(e==dupFNErr){e=info(part,&cat);if(!e&&!(cat.hFileInfo.ioFlAttrib&16))e=dirNFErr;}if(e)return e;}
 snprintf(part,sizeof part,"%.240setc:profile",root_path);pstr(name,part);e=HCreate(0,0,name,'S6UX','TEXT');if(e==dupFNErr)return 0;if(e)return e;short ref;e=HOpen(0,0,name,fsWrPerm,&ref);if(!e){static const char profile[]="# Clarice startup profile; sourced once at launch.\nexport PATH=/bin:/usr/bin\n# Optional local hostname (no networking required):\n# export HOSTNAME=macplus\n";long n=sizeof profile-1;e=FSWrite(ref,&n,profile);if(!e&&n!=sizeof profile-1)e=ioErr;FSClose(ref);}if(e)HDelete(0,0,name);FlushVol(0,0);return e;
}
