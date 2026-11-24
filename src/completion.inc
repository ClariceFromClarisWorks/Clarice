/* Streaming completion: first Tab computes a common prefix, second Tab offers
 * bounded pages. No directory entries or executable images are retained. */
#define COMPLETION_PAGE 16
#define COMPLETION_SCAN_BATCH 8
typedef struct {const char *prefix;char common[PATH_CAP];int count,show,shown,stop,page_count,page_chars;} Completion;
static int completion_prefix(const char *s,const char *p){for(;*p;s++,p++)if(!*s||tolower((unsigned char)*s)!=tolower((unsigned char)*p))return 0;return 1;}
static void completion_item(Completion *c,const char *name){
 if(c->stop||strlen(name)>=PATH_CAP||!completion_prefix(name,c->prefix))return;
 if(!c->count)strcpy(c->common,name);else{int i=0;while(c->common[i]&&name[i]&&tolower((unsigned char)c->common[i])==tolower((unsigned char)name[i]))i++;c->common[i]=0;}
 c->count++;
 if(!c->show){if(c->count>1&&strlen(c->common)<=strlen(c->prefix))c->stop=1;return;}
 if(c->shown&&(c->page_count==COMPLETION_PAGE||c->page_chars+(int)strlen(name)+2>app_columns()*4)){if(!app_completion_more()){c->stop=app_completion_poll()?2:1;return;}c->page_count=c->page_chars=0;}
 if(!c->shown)term_puts("\n");term_puts(name);term_puts("  ");c->shown++;c->page_count++;c->page_chars+=strlen(name)+2;
}
static void complete_functions(Completion *);
static int builtin_name(const char *s){int i;for(i=0;commands[i];i++)if(!strcmp(commands[i],s))return 1;return 0;}
static void complete_directory(const char *dir,Completion *c,int command){
 char native[PATH_CAP],canonical[PATH_CAP],candidate[100],full[PATH_CAP];short vol;long id;int index=1,e;
 if(c->stop||resolve_ex(dir,native,canonical,1))return;
 if(!strcmp(canonical,"/mnt")){char name[32];while(!c->stop&&!volume_name(index++,name)){if(app_completion_poll()){c->stop=2;break;}if(encode_name(name,candidate,sizeof candidate))continue;strcat(candidate,"/");completion_item(c,candidate);}return;}
 if(directory(dir,&vol,&id))return;
 while(!c->stop){
  /* Yield on scanned entries, including nonmatches, not only displayed names. */
  if(index>1&&!((index-1)%COMPLETION_SCAN_BATCH)&&app_completion_poll()){c->stop=2;break;}
  if(index>32767){c->stop=2;break;} /* Native signed catalog index. */
  CInfoPBRec pb;unsigned char name[256];memset(&pb,0,sizeof pb);pb.hFileInfo.ioNamePtr=name;pb.hFileInfo.ioVRefNum=vol;pb.hFileInfo.ioDirID=id;pb.hFileInfo.ioFDirIndex=index++;e=PBGetCatInfoSync(&pb);if(e){if(e!=fnfErr)c->stop=2;break;}
  name[name[0]+1]=0;if(encode_name((char*)name+1,candidate,sizeof candidate)||!completion_prefix(candidate,c->prefix))continue;
  if(strlen(dir)+strlen(candidate)+2>=sizeof full)continue;strcpy(full,dir);strcat(full,"/");strcat(full,candidate);
  if(command){char selected[PATH_CAP];if(builtin_name(candidate)||(pb.hFileInfo.ioFlAttrib&16)||!program_kind(full))continue;if(command_path(candidate,selected,0)!=1||strcmp(selected,full))continue;}
  else{int folder=pb.hFileInfo.ioFlAttrib&16;if(!folder&&is_link(&pb)){short v;long d;folder=!directory(full,&v,&d);}if(folder)strcat(candidate,"/");}
  completion_item(c,candidate);
 }
}
static char previous[LINE_CAP];static int previous_cursor=-1;
void shell_completion_reset(void){previous_cursor=-1;}
int shell_complete(char *line,int *cursor){
 Completion c;memset(&c,0,sizeof c);c.show=previous_cursor==*cursor&&!strcmp(previous,line);
 char token[PATH_CAP],dir[PATH_CAP],encoded[LINE_CAP];int start=0,q=0,j=0,i,cmd=1;const char *base;
 for(i=0;i<*cursor;i++){char ch=line[i];if(ch=='\\'&&q!=1){i++;continue;}if(ch=='\''&&q!=2){q=q==1?0:1;continue;}if(ch=='"'&&q!=1){q=q==2?0:2;continue;}if(!q&&(ch==' '||ch=='\t')){start=i+1;cmd=0;}}
 q=0;for(i=start;i<*cursor;i++){char ch=line[i];if(ch=='\''&&q!=2){q=q==1?0:1;continue;}if(ch=='"'&&q!=1){q=q==2?0:2;continue;}if(ch=='\\'&&q!=1&&i+1<*cursor)ch=line[++i];if(j>=PATH_CAP-2){previous_cursor=-1;return 0;}token[j++]=ch;}token[j]=0;
 if(cmd&&!strchr(token,'/')){c.prefix=token;for(i=0;commands[i]&&!c.stop;i++)completion_item(&c,commands[i]);complete_functions(&c);const char *path=env_get("PATH");while(!c.stop&&path_entry(&path,dir))complete_directory(dir,&c,1);}
 else{char *slash=strrchr(token,'/');if(slash){size_t n=(size_t)(slash-token)+1;memcpy(dir,token,n);dir[n]=0;base=slash+1;}else{strcpy(dir,".");base=token;}c.prefix=base;complete_directory(dir,&c,0);
  if(c.count&&slash){size_t n=(size_t)(slash-token)+1;if(n+strlen(c.common)>=PATH_CAP){previous_cursor=-1;return 0;}memmove(c.common+n,c.common,strlen(c.common)+1);memcpy(c.common,token,n);}}
 if(c.shown)term_puts("\n");
 if(c.stop==2){previous_cursor=-1;return -1;}if(!c.count){previous_cursor=-1;return 0;}if(c.show)return c.count;
 /* Normalize mixed quoting to safe unquoted escapes. If the cursor is inside
    a quote, retain that quote so the unchanged suffix remains meaningful. */
 j=0;if(q)encoded[j++]=q==1?'\'':'"';
 for(i=0;c.common[i];i++){unsigned char ch=c.common[i];const char *escape=q==2?"\\\"$`":q==1?"":" \t\\'\"$`><&;|()*?[]{}";
  if(j+7>=LINE_CAP){previous_cursor=-1;return 0;}
  if(q==1&&ch=='\''){encoded[j++]='\'';encoded[j++]='\\';encoded[j++]='\'';encoded[j++]='\'';}else{if(strchr(escape,ch))encoded[j++]='\\';encoded[j++]=ch;}}
 int tail=*cursor;
 if(c.count==1&&j&&encoded[j-1]!='/'){
  if(q){encoded[j++]=q==1?'\'':'"';if(line[tail]==encoded[j-1])tail++;}encoded[j++]=' ';
 }
 encoded[j]=0;if(start+j+(int)strlen(line+tail)>=LINE_CAP){previous_cursor=-1;return 0;}
 memmove(line+start+j,line+tail,strlen(line+tail)+1);memcpy(line+start,encoded,j);*cursor=start+j;strcpy(previous,line);previous_cursor=*cursor;return c.count;
}
