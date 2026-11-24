#ifdef CLARICE_LIST_DIAGNOSTICS
static unsigned long ls_phase[5]; /* resolve, catalog, format, emit, yield */
#define LS_START unsigned long ls_stamp=TickCount()
#define LS_PHASE(n) (ls_phase[n]+=TickCount()-ls_stamp,ls_stamp=TickCount())
static void ls_emit(const char *s,long n){unsigned long t=TickCount();emit(s,n);ls_phase[3]+=TickCount()-t;}
static int ls_yield(int paged){unsigned long t=TickCount();int rc=paged?app_page():app_yield();ls_phase[4]+=TickCount()-t;return rc;}
#else
#define LS_START ((void)0)
#define LS_PHASE(n) ((void)0)
#define ls_emit emit
#define ls_yield(paged) ((paged)?app_page():app_yield())
#endif
/* Streaming, row-major output. One terminal row plus an optional file-output buffer, never a directory-sized
   array or preliminary metadata pass. Catalog order is retained. */
typedef struct {int detail,human,paged,multi,used,presented;char row[133],created[17];unsigned long created_seconds;int created_valid;char *buffer;unsigned buffered;} Listing;
#define LIST_BUFFER_SIZE 4096
static void list_drain(Listing *l){if(l->buffered){if(!output_error)ls_emit(l->buffer,l->buffered);l->buffered=0;}}
static void list_write(Listing *l,const char *s,unsigned n){
 if(!l->buffer){ls_emit(s,n);return;}
 if(n>LIST_BUFFER_SIZE-l->buffered)list_drain(l);
 if(output_error)return;
 if(n>LIST_BUFFER_SIZE){ls_emit(s,n);return;}
 memcpy(l->buffer+l->buffered,s,n);l->buffered+=n;
}
static int list_flush(Listing *l){if(l->used){l->row[l->used++]='\n';list_write(l,l->row,l->used);l->used=0;if(!l->presented){term_present();l->presented=1;}}if(output_error)return 1;return (l->paged?app_page():app_yield())?130:0;}
static int list_item(Listing *l,const char *name,const CInfoPBRec *pb){
 if(l->detail<0)return app_yield()?130:0;
 LS_START;
 int dir=!pb||(pb->hFileInfo.ioFlAttrib&16),e;
 if(l->detail){char meta[PATH_CAP+128],data[32],resource[32],kind[6],type[10];unsigned int flags=pb?pb->hFileInfo.ioFlFndrInfo.fdFlags:0;int j;
  kind[0]=dir?'d':is_link(pb)?'l':'-';kind[1]=pb&&(pb->hFileInfo.ioFlAttrib&1)?'L':'-';kind[2]=flags&0x4000?'I':'-';kind[3]=flags&0x1000?'N':'-';kind[4]=flags&0x2000?'B':'-';kind[5]=0;
  for(j=0;j<8;j++){unsigned char c=dir?'-':((unsigned char*)&pb->hFileInfo.ioFlFndrInfo)[j];type[j+(j>=4)]=(c>=32&&c<127)?c:'.';}type[4]=' ';type[9]=0;
  /* Avoid general printf parsing for every catalog entry. */
  int k=0,n;static const char hex[]= "0123456789ABCDEF";k=l->detail==2?5:3;memcpy(meta,kind,k);meta[k++]=' ';
  if(l->detail==2){for(j=12;j>=0;j-=4)meta[k++]=pb?hex[(flags>>j)&15]:'-';meta[k++]=' ';}
  if(!pb||dir){memset(meta+k,' ',22);k+=22;}
  else if(!l->human){format_decimal_field(meta+k,(unsigned long)pb->hFileInfo.ioFlLgLen);k+=10;meta[k++]=' ';format_decimal_field(meta+k,(unsigned long)pb->hFileInfo.ioFlRLgLen);k+=10;meta[k++]=' ';}
  else{format_size(data,(unsigned long)pb->hFileInfo.ioFlLgLen,1);format_size(resource,(unsigned long)pb->hFileInfo.ioFlRLgLen,1);const char *sizes[2]={data,resource};for(j=0;j<2;j++){n=strlen(sizes[j]);if(n<10){memset(meta+k,' ',10-n);k+=10-n;}memcpy(meta+k,sizes[j],n);k+=n;meta[k++]=' ';}}
  memcpy(meta+k,type,9);k+=9;meta[k++]=' ';if(pb){unsigned long seconds=dir?(unsigned long)pb->dirInfo.ioDrCrDat:(unsigned long)pb->hFileInfo.ioFlCrDat;
   /* One invocation-local value, exact timestamp key; no directory cache. */
   if(!l->created_valid||l->created_seconds!=seconds){format_created(l->created,seconds);l->created_seconds=seconds;l->created_valid=1;}memcpy(meta+k,l->created,16);}else memset(meta+k,' ',16);k+=16;meta[k++]=' ';n=strlen(name);memcpy(meta+k,name,n);k+=n;if(dir&&strcmp(name,".")&&strcmp(name,".."))meta[k++]='/';meta[k++]='\n';LS_PHASE(2);list_write(l,meta,k);return output_error?1:ls_yield(l->paged)?130:0;
 }
 /* Names arrive encoded; append the directory marker without printf or a
    second length scan. Preserve the historical 103-byte operand display bound. */
 char shown[104];int len=0,width=app_columns();
 while(name[len]&&len<103){shown[len]=name[len];len++;}
 if(dir&&strcmp(name,".")&&strcmp(name,"..")&&len<103)shown[len++]='/';
 if(!l->multi){shown[len++]='\n';list_write(l,shown,len);return output_error?1:ls_yield(l->paged)?130:0;}
 /* Fixed 16-cell stops keep streaming rows aligned without collecting names. */
 int at=l->used?((l->used+2+15)/16)*16:0;
 if(l->used&&at+len>width){if((e=list_flush(l)))return e;at=0;}
 if(len>width){shown[len++]='\n';list_write(l,shown,len);return output_error?1:ls_yield(l->paged)?130:0;}
 memset(l->row+l->used,' ',at-l->used);memcpy(l->row+at,shown,len);l->used=at+len;
 if(l->used==width)return list_flush(l);return app_yield()?130:0;
}
static unsigned long list_ticks,list_first,list_entries;
static int list_walk(const char *s,int detail,int all,Listing *l){
#ifdef CLARICE_LIST_DIAGNOSTICS
 memset(ls_phase,0,sizeof ls_phase);
#endif
 LS_START;unsigned long started=TickCount();char np[PATH_CAP],up[PATH_CAP];CInfoPBRec current;int e=cli_resolve_info(s,np,up,&current),i;
 /* /mnt is virtual even when its backing folder is absent. Only this missing
    metadata case needs the creation-capable resolver; never use stale output. */
 if(e==fnfErr){int missing=e;if(!resolve_ex(s,np,up,1)&&!strcmp(up,"/mnt"))e=0;else e=missing;}
 list_first=list_entries=0;app_page_begin();if(e){error("ls",e);return 1;}
 int mounts=!strcmp(up,"/mnt");if(!mounts){
  if(!(current.hFileInfo.ioFlAttrib&16)){e=list_item(l,s,&current);return e?e:list_flush(l);}
 }
 if(all&&detail>=0){if((e=list_item(l,".",mounts?0:&current)))return e;
  CInfoPBRec parent;char path[PATH_CAP],native[PATH_CAP];strcpy(path,up);parent_path(path);const CInfoPBRec *p=0;
  if(detail>0&&strcmp(path,"/mnt")&&!cli_resolve_info(path,native,0,&parent))p=&parent;
  if((e=list_item(l,"..",p)))return e;
 }
 LS_PHASE(0);
 short v=mounts?0:current.dirInfo.ioVRefNum;long id=mounts?0:current.dirInfo.ioDrDirID;
 /* Reuse the synchronous parameter block. HFS overwrites ioDirID, so restore
    the explicit volume, parent ID and index before every call. */
 CInfoPBRec pb;unsigned char name[256];memset(&pb,0,sizeof pb);
 for(i=1;;i++){
  char raw[32],shown[100];const CInfoPBRec *entry=&pb;
  if(mounts){e=volume_name(i,raw);if(e==nsvErr)break;if(!e)e=encode_name(raw,shown,sizeof shown);if(e){error("ls",e);return 1;}entry=0;
   if(detail>0){char root[40];snprintf(root,sizeof root,"%s:",raw);if(!info(root,&pb))entry=&pb;}
  }else{pb.hFileInfo.ioNamePtr=name;pb.hFileInfo.ioVRefNum=v;pb.hFileInfo.ioDirID=id;pb.hFileInfo.ioFDirIndex=i;e=PBGetCatInfoSync(&pb);if(e==fnfErr)break;if(e){error("ls",e);return 1;}
   if(!all&&((name[0]&&name[1]=='.')||(pb.hFileInfo.ioFlFndrInfo.fdFlags&0x4000))){if(app_yield())return 130;continue;}
   /* HFS names are at most 31 bytes; reject malformed results before use. */
   if(name[0]>31){error("ls",bdNamErr);return 1;}name[name[0]+1]=0;if(encode_name((char*)name+1,shown,sizeof shown))return 1;
  }
  LS_PHASE(1);list_entries++;if((e=list_item(l,shown,entry)))return e;
  if(list_entries==1){if(!l->multi||detail){term_present();list_first=TickCount()-started;}}
#ifdef CLARICE_LIST_DIAGNOSTICS
  ls_stamp=TickCount();
#endif
 }
 e=list_flush(l);list_ticks=TickCount()-started;return e;
}
static int list_impl(const char *s,int detail,int all,int paged,int human,int one){
 Listing l;memset(&l,0,sizeof l);l.detail=detail;l.human=human;l.paged=paged;l.multi=!one&&output_ref<0&&!script_depth;
 /* Buffer only native file output, not terminal/session/device streams. Failure
    to obtain this optional working memory retains direct streaming. */
 if(output_ref>=0&&detail>=0)l.buffer=cli_alloc(LIST_BUFFER_SIZE);
 int e=list_walk(s,detail,all,&l);
 /* Preserve already produced output on catalog errors and cancellation. Never
    retry a failed write; stream_write has already handled short writes. */
 list_drain(&l);if(l.buffer)cli_free(l.buffer);
 return e==130?130:output_error?1:e;
}
/* Batch bulk paints, but keep event polling at two ticks and page/first-entry
   presentation immediate. Restore the ordinary cadence on every exit. */
static int list(const char *s,int detail,int all,int paged,int human,int one){app_output_batch(1);int e=list_impl(s,detail,all,paged,human,one);app_output_batch(0);return e;}
/* Options are validated first. Revisit borrowed argv for operands instead of
   copying them into a fixed stack array; no allocation or argv mutation. */
static const char *listing_operand(int argc,char **av,int *index,int *options){
 while(*index<argc){const char *path=av[(*index)++];
  if(*options&&!strcmp(path,"--")){*options=0;continue;}
  if(*options&&path[0]=='-'&&path[1])continue;
  return path;
 }return NULL;
}
static int ls_command(int argc,char **av){
 if(argc==2&&!strcmp(av[1],"--help"))return help_output(1,
  "Usage:\n"
  "  ls [-lah1P] [--full] [--legend] [--] [path ...]\n"
  "List HFS entries in catalog order.\n"
  "Options:\n"
  "  -l\tShow kind and flags, both fork sizes, type and creator, creation date, and name.\n"
  "  -a\tInclude Finder-invisible entries, dot names, and the dot and dot-dot entries. Explicit operands are shown even when hidden.\n"
  "  -h\tUse KiB/MiB for fork sizes; zero indicates an empty fork.\n"
  "  -1\tShow one entry per line.\n"
  "  -P\tDisable listing paging if enabled.\n"
  "  --full\tAdd N/B flags and raw Finder hex.\n"
  "  --legend\tAdd headings and explanations. Implies -l.\n"
  "Long listing:\n"
  "  Kind: d directory, l Clarice link, - ordinary file.\n"
  "  Flags: L locked, I invisible, N name-locked, B bundle (OS-dependent), - clear.\n"
  "  Fork lengths are logical bytes. Directory and unavailable virtual sizes are blank.\n"
  "  Creation date comes from HFS, YYYY-MM-DD HH:MM, when available.\n"
  "  Type and creator are stored codes. A dot represents a nonprintable byte. Stored ???? is literal. Directories show ---- ----.\n"
  "HFS has no Unix owners, groups or access permissions.\n"
  "Example:\n"
  "  ls --legend /mnt\n");
int detail=0,all=0,paged=1,human=0,one=0,legend=0,options=1,n=0,i;int operand=1,operand_options=1;
 for(i=1;i<argc;i++){if(options&&!strcmp(av[i],"--legend")){legend=1;continue;}if(options&&!strcmp(av[i],"--full")){detail=2;continue;}if(options&&!strcmp(av[i],"--")){options=0;continue;}if(options&&av[i][0]=='-'&&av[i][1]){const char *f=av[i]+1;while(*f){switch(*f++){case 'l':if(!detail)detail=1;break;case 'a':all=1;break;case 'h':human=1;break;case '1':one=1;break;case 'P':paged=0;break;default:errsay("usage: ls [-lah1P] [--full] [--legend] [--] [path ...]\n");return 2;}}}else n++;}
 if(legend){if(!detail)detail=1;
  say("FLG: kind (d directory, l link, - file), L locked, I invisible.\n"
      "DATA/RESOURCE: logical fork sizes; blank for directories.\n"
      "TYPE/CREA: Macintosh type/creator; '.' marks nonprintable bytes.\n"
      "CREATED: HFS creation date/time. NAME: encoded filename.\n");
  if(human)say("Sizes use KiB/MiB; zero means an empty fork.\n");
  else say("Sizes are bytes; zero means an empty fork.\n");
  if(detail==2)say("FLAGS also has N name-locked, B bundle; HEX is raw Finder flags.\n");
  if(output_error)return 1;
 }
 int result=0;for(i=0;i<(n?n:1);i++){const char *path=n?listing_operand(argc,av,&operand,&operand_options):".";if(n>1){if(i)say("\n");say(path);say(":\n");}if(legend){say(detail==2?"FLAGS HEX  ":"FLG ");say("      DATA   RESOURCE TYPE CREA CREATED          NAME\n");if(output_error)return 1;}int e=list(path,detail,all,paged&&output_ref<0&&!script_depth,human,one);if(e==130)return e;if(e)result=e;if(output_error)return 1;}return result;
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
static int df_command(int argc,char **av){int human=0,options=1,n=0,i,result=0;int operand=1,operand_options=1;
 for(i=1;i<argc;i++){if(options&&!strcmp(av[i],"--")){options=0;continue;}if(options&&av[i][0]=='-'&&av[i][1]){const char *p=av[i]+1;while(*p){if(*p=='h')human=1;else if(*p=='k')human=0;else{errsay("usage: df [-h|-k] [--] [path ...]\n");return 2;}p++;}}else n++;}
 say(human?"        Size         Used        Avail Use% Mounted on\n":"     Size-KB      Used-KB     Avail-KB Use% Mounted on\n");
 for(i=n?0:1;;i++){
  HParamBlockRec pb;unsigned char name[256];memset(&pb,0,sizeof pb);pb.volumeParam.ioNamePtr=name;
  int e;
  if(n){if(i>=n)break;char native[PATH_CAP];CInfoPBRec item;e=resolve(listing_operand(argc,av,&operand,&operand_options),native);if(!e)e=info(native,&item);if(e){error("df",e);result=1;continue;}
   char *colon=strchr(native,':');if(!colon){result=1;continue;}*colon=0;
   /* Enumerate by index: full-path catalog calls need not return a resolved
      volume reference in their input ioVRefNum field. */
   int index;for(index=1;;index++){memset(&pb,0,sizeof pb);pb.volumeParam.ioNamePtr=name;pb.volumeParam.ioVolIndex=index;e=PBHGetVInfoSync(&pb);if(e)break;char label[32];memcpy(label,name+1,name[0]);label[name[0]]=0;if(same_name(label,native))break;}
  }else{pb.volumeParam.ioVolIndex=i;e=PBHGetVInfoSync(&pb);}
  if(!n&&e==nsvErr)break;if(e){error("df",e);if(!n)return 1;result=1;continue;}
  e=df_row(&pb,human);if(e==130)return e;if(e)result=e;
 }return result;
}
