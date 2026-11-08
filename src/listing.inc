/* Streaming, row-major output. One row of storage, never a directory-sized
   array or preliminary metadata pass. Catalog order is retained. */
typedef struct {int detail,human,paged,multi,used,presented;char row[132];} Listing;
static int list_flush(Listing *l){if(l->used){emit(l->row,l->used);say("\n");l->used=0;if(!l->presented){term_present();l->presented=1;}}if(output_error)return 1;return (l->paged?app_page():app_yield())?130:0;}
static int list_item(Listing *l,const char *name,const CInfoPBRec *pb){
 if(l->detail<0)return app_yield()?130:0;
 int dir=!pb||(pb->hFileInfo.ioFlAttrib&16),e;
 if(l->detail){char meta[PATH_CAP+128],data[32],resource[32],kind[6],type[10],created[17];unsigned int flags=pb?pb->hFileInfo.ioFlFndrInfo.fdFlags:0;int j;
  kind[0]=dir?'d':is_link(pb)?'l':'-';kind[1]=pb&&(pb->hFileInfo.ioFlAttrib&1)?'L':'-';kind[2]=flags&0x4000?'I':'-';kind[3]=flags&0x1000?'N':'-';kind[4]=flags&0x2000?'B':'-';kind[5]=0;
  if(pb&&!dir){if(l->human){format_size(data,(unsigned long)pb->hFileInfo.ioFlLgLen,1);format_size(resource,(unsigned long)pb->hFileInfo.ioFlRLgLen,1);}else{format_decimal32(data,(unsigned long)pb->hFileInfo.ioFlLgLen);format_decimal32(resource,(unsigned long)pb->hFileInfo.ioFlRLgLen);}}else{data[0]=resource[0]=0;}
  if(l->detail==2)for(j=0;j<8;j++){unsigned char c=dir?'-':((unsigned char*)&pb->hFileInfo.ioFlFndrInfo)[j];type[j+(j>=4)]=(c>=32&&c<127)?c:'.';}type[4]=' ';type[9]=0;
  /* Avoid general printf parsing for every catalog entry. */
  int k=0,n;static const char hex[]= "0123456789ABCDEF";k=l->detail==2?5:3;memcpy(meta,kind,k);meta[k++]=' ';
  if(l->detail==2){for(j=12;j>=0;j-=4)meta[k++]=pb?hex[(flags>>j)&15]:'-';meta[k++]=' ';}
  const char *sizes[2]={data,resource};for(j=0;j<2;j++){n=strlen(sizes[j]);if(n<10){memset(meta+k,' ',10-n);k+=10-n;}memcpy(meta+k,sizes[j],n);k+=n;meta[k++]=' ';}
  if(l->detail==2){memcpy(meta+k,type,9);k+=9;meta[k++]=' ';}if(pb){format_created(created,dir?(unsigned long)pb->dirInfo.ioDrCrDat:(unsigned long)pb->hFileInfo.ioFlCrDat);memcpy(meta+k,created,16);}else memset(meta+k,' ',16);k+=16;meta[k++]=' ';n=strlen(name);memcpy(meta+k,name,n);k+=n;if(dir&&strcmp(name,".")&&strcmp(name,".."))meta[k++]='/';meta[k++]='\n';emit(meta,k);return output_error?1:(l->paged?app_page():app_yield())?130:0;
 }
 char shown[104];snprintf(shown,sizeof shown,"%s%s",name,dir&&strcmp(name,".")&&strcmp(name,"..")?"/":"");int len=strlen(shown),width=app_columns();
 if(!l->multi){say(shown);say("\n");return output_error?1:(l->paged?app_page():app_yield())?130:0;}
 /* Fixed 16-cell stops keep streaming rows aligned without collecting names. */
 int at=l->used?((l->used+2+15)/16)*16:0;
 if(l->used&&at+len>width){if((e=list_flush(l)))return e;at=0;}
 if(len>width){say(shown);say("\n");return output_error?1:(l->paged?app_page():app_yield())?130:0;}
 memset(l->row+l->used,' ',at-l->used);memcpy(l->row+at,shown,len);l->used=at+len;
 if(l->used==width)return list_flush(l);return app_yield()?130:0;
}
static unsigned long list_ticks,list_first,list_entries;
static int list_impl(const char *s,int detail,int all,int paged,int human,int one){
 unsigned long started=TickCount();char np[PATH_CAP],up[PATH_CAP];CInfoPBRec current;int e=resolve_ex(s,np,up,1),i;
 Listing l;memset(&l,0,sizeof l);l.detail=detail;l.human=human;l.paged=paged;l.multi=!one&&output_ref<0&&!script_depth;
 list_first=list_entries=0;app_page_begin();if(e){error("ls",e);return 1;}
 int mounts=!strcmp(up,"/mnt");if(!mounts){e=info(np,&current);if(e){error("ls",e);return 1;}
  if(!(current.hFileInfo.ioFlAttrib&16)){e=list_item(&l,s,&current);return e?e:list_flush(&l);}
 }
 if(all&&detail>=0){if((e=list_item(&l,".",mounts?0:&current)))return e;
  CInfoPBRec parent;char path[PATH_CAP],native[PATH_CAP];strcpy(path,up);parent_path(path);const CInfoPBRec *p=0;
  if(strcmp(path,"/mnt")&&!resolve(path,native)&&!info(native,&parent))p=&parent;
  if((e=list_item(&l,"..",p)))return e;
 }
 short v=mounts?0:current.dirInfo.ioVRefNum;long id=mounts?0:current.dirInfo.ioDrDirID;
 for(i=1;;i++){
  CInfoPBRec pb;unsigned char name[256];char raw[32],shown[100];const CInfoPBRec *entry=&pb;
  if(mounts){e=volume_name(i,raw);if(e==nsvErr)break;if(!e)e=encode_name(raw,shown,sizeof shown);if(e){error("ls",e);return 1;}entry=0;
   if(detail>0){char root[40];snprintf(root,sizeof root,"%s:",raw);if(!info(root,&pb))entry=&pb;}
  }else{memset(&pb,0,sizeof pb);pb.hFileInfo.ioNamePtr=name;pb.hFileInfo.ioVRefNum=v;pb.hFileInfo.ioDirID=id;pb.hFileInfo.ioFDirIndex=i;e=PBGetCatInfoSync(&pb);if(e==fnfErr)break;if(e){error("ls",e);return 1;}
   if(!all&&((name[0]&&name[1]=='.')||(pb.hFileInfo.ioFlFndrInfo.fdFlags&0x4000))){if(app_yield())return 130;continue;}memcpy(raw,name+1,name[0]);raw[name[0]]=0;if(encode_name(raw,shown,sizeof shown))return 1;
  }
  list_entries++;if((e=list_item(&l,shown,entry)))return e;
  if(list_entries==1){if(!l.multi||detail){term_present();list_first=TickCount()-started;}}
 }
 e=list_flush(&l);list_ticks=TickCount()-started;return e;
}
/* Batch bulk paints, but keep event polling at two ticks and page/first-entry
   presentation immediate. Restore the ordinary cadence on every exit. */
static int list(const char *s,int detail,int all,int paged,int human,int one){app_output_batch(1);int e=list_impl(s,detail,all,paged,human,one);app_output_batch(0);return e;}
static int ls_command(int argc,char **av){
 if(argc==2&&!strcmp(av[1],"--help")){say(
 "usage: ls [-lah1P] [--full] [--] [path ...]\n"
 "-l columns: kind+LI  data-bytes  resource-bytes  created  name\n"
 "Created: YYYY-MM-DD HH:MM, from the HFS catalog.\n"
 "Kind: d directory, l Clarice link, - ordinary file.\n"
 "L: locked; I: Finder-invisible; -: flag clear.\n"
 "Fork sizes are logical bytes; -h uses KiB/MiB; zero is empty.\n"
 "Directories have blank fork sizes: these are not applicable.\n"
 "Virtual entries leave unavailable sizes/dates blank.\n"
 "--full adds N/B flag bits, raw Finder hex, type and creator.\n"
 "N: name-locked bit; B: bundle bit (OS-dependent meaning).\n"
 "Type/creator: stored codes; '.' replaces nonprintable bytes.\n"
 "Stored ???? is literal metadata, not our placeholder.\n"
 "Directory type/creator use ---- ---- (not applicable).\n"
 "-a includes Finder-invisible entries, dot names, . and ..\n"
 "Explicit path operands are shown even when hidden.\n"
 "-1 one entry per line; -P no paging; catalog order.\n"
 "No invented owners, groups or Unix access permissions.\n");return output_error?1:0;}
int detail=0,all=0,paged=1,human=0,one=0,options=1,n=0,i;const char *paths[MAX_ARGS];
 for(i=1;i<argc;i++){if(options&&!strcmp(av[i],"--full")){detail=2;continue;}if(options&&!strcmp(av[i],"--")){options=0;continue;}if(options&&av[i][0]=='-'&&av[i][1]){const char *f=av[i]+1;while(*f){switch(*f++){case 'l':if(!detail)detail=1;break;case 'a':all=1;break;case 'h':human=1;break;case '1':one=1;break;case 'P':paged=0;break;default:errsay("usage: ls [-lah1P] [--full] [--] [path ...]\n");return 2;}}}else paths[n++]=av[i];}
 if(!n)paths[n++]=".";int result=0;for(i=0;i<n;i++){if(n>1){if(i)say("\n");say(paths[i]);say(":\n");}int e=list(paths[i],detail,all,paged&&output_ref<0&&!script_depth,human,one);if(e==130)return e;if(e)result=e;if(output_error)return 1;}return result;
}
static int df_row(HParamBlockRec *pb,int human){
 char size[32],used[32],free_space[32],line[224],raw[32],mount[100];unsigned long blocks=(unsigned short)pb->volumeParam.ioVNmAlBlks,free_blocks=(unsigned short)pb->volumeParam.ioVFrBlk,block_size=(unsigned long)pb->volumeParam.ioVAlBlkSiz;
 if(free_blocks>blocks){errsay("df: inconsistent volume allocation counts\n");return 1;}
 unsigned long long total=(unsigned long long)blocks*block_size,available=(unsigned long long)free_blocks*block_size;
 format_size(size,human?total:(total+1023)>>10,human);format_size(used,human?total-available:(total-available+1023)>>10,human);format_size(free_space,human?available:(available+1023)>>10,human);
 unsigned char *p=pb->volumeParam.ioNamePtr;memcpy(raw,p+1,p[0]);raw[p[0]]=0;if(encode_name(raw,mount,sizeof mount))return 1;
 unsigned long percent=blocks?((blocks-free_blocks)*100+blocks-1)/blocks:0;
 snprintf(line,sizeof line,"%12s %12s %12s %3lu%% /mnt/%s\n",size,used,free_space,percent,mount);say(line);return output_error?1:app_yield()?130:0;
}
static int df_command(int argc,char **av){int human=0,options=1,n=0,i,result=0;const char *paths[MAX_ARGS];
 for(i=1;i<argc;i++){if(options&&!strcmp(av[i],"--")){options=0;continue;}if(options&&av[i][0]=='-'&&av[i][1]){const char *p=av[i]+1;while(*p){if(*p=='h')human=1;else if(*p=='k')human=0;else{errsay("usage: df [-h|-k] [--] [path ...]\n");return 2;}p++;}}else paths[n++]=av[i];}
 say(human?"        Size         Used        Avail Use% Mounted on\n":"     Size-KB      Used-KB     Avail-KB Use% Mounted on\n");
 for(i=n?0:1;;i++){
  HParamBlockRec pb;unsigned char name[256];memset(&pb,0,sizeof pb);pb.volumeParam.ioNamePtr=name;
  int e;
  if(n){if(i>=n)break;char native[PATH_CAP];CInfoPBRec item;e=resolve(paths[i],native);if(!e)e=info(native,&item);if(e){error("df",e);result=1;continue;}
   char *colon=strchr(native,':');if(!colon){result=1;continue;}*colon=0;
   /* Enumerate by index: full-path catalog calls need not return a resolved
      volume reference in their input ioVRefNum field. */
   int index;for(index=1;;index++){memset(&pb,0,sizeof pb);pb.volumeParam.ioNamePtr=name;pb.volumeParam.ioVolIndex=index;e=PBHGetVInfoSync(&pb);if(e)break;char label[32];memcpy(label,name+1,name[0]);label[name[0]]=0;if(same_name(label,native))break;}
  }else{pb.volumeParam.ioVolIndex=i;e=PBHGetVInfoSync(&pb);}
  if(!n&&e==nsvErr)break;if(e){error("df",e);if(!n)return 1;result=1;continue;}
  e=df_row(&pb,human);if(e==130)return e;if(e)result=e;
 }return result;
}
