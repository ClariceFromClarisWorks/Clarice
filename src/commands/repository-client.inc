/* Repository orchestration only. Native HFS installation and the independent
 * Core helper remain the owners of replacement/rollback of individual files. */
#include "../update/repository.h"
#define REPO_CATALOG "/etc/clarice/installed.catalog"
#define REPO_MARKER "/etc/clarice/update.incomplete"
extern int cli_appinstall(const char *,const char *,unsigned long);
typedef struct RepoItem {struct RepoItem *next;RepoRecord wanted,installed;int known,needed;} RepoItem;
typedef struct {short ref;char bytes[1024];unsigned pos,end;long complete;int eof;} RepoReader;
static int repo_next(RepoReader *r,char *line,unsigned cap){
 unsigned used=0;for(;;){
  if(r->pos==r->end){long n=sizeof r->bytes;int e=FSRead(r->ref,&n,r->bytes);if(e&&e!=eofErr)return e;r->pos=0;r->end=n;if(!n){r->eof=1;line[used]=0;return used?2:0;}}
  char c=r->bytes[r->pos++];if(!c||c=='\r')return paramErr;
  if(c=='\n'){line[used]=0;r->complete+=used+1;return 1;}
  if(used+1>=cap)return paramErr;line[used++]=c;
 }
}
static RepoItem *repo_find(RepoItem *list,const RepoRecord *r){for(;list;list=list->next)if(list->wanted.core==r->core&&!strcmp(list->wanted.name,r->name))return list;return 0;}
static int repo_read_index(const char *path,RepoItem **list){
 RepoReader r;memset(&r,0,sizeof r);int e=open_path(path,fsRdPerm,&r.ref);if(e)return e;
 char line[256];int n=repo_next(&r,line,sizeof line),cores=0;
 if(n!=1||strcmp(line,"CLARICE-REPOSITORY 1")){e=paramErr;goto end;}
 while((n=repo_next(&r,line,sizeof line))==1){
  RepoRecord rec;if(repo_record(line,&rec,0)||repo_find(*list,&rec)){e=paramErr;break;}
  RepoItem *item=cli_alloc(sizeof *item);if(!item){e=memFullErr;break;}memset(item,0,sizeof *item);item->wanted=rec;item->next=*list;*list=item;cores+=rec.core;
  if(app_yield()){e=userCanceledErr;break;}
 }
 if(!e&&(n!=0||cores!=1))e=n<0?n:paramErr;
end: {int x=FSClose(r.ref);return e?e:x;}
}
/* Complete newline-terminated records are durable catalog entries. A torn last
 * append is ignored, then truncated before the next append. Last record wins. */
static int repo_catalog(RepoItem *list,const RepoRecord *append){
 RepoReader r;memset(&r,0,sizeof r);char native[PATH_CAP],canonical[PATH_CAP],line[256];
 int e=checked(REPO_CATALOG,native,canonical,0);if(e)return e;
 e=open_path(REPO_CATALOG,append?fsRdWrPerm:fsRdPerm,&r.ref);
 if(e==fnfErr&&!append)return 0;
 if(e==fnfErr){e=cli_native_create(native,'TEXT','CLRC');if(!e)e=cli_native_open(native,0,1,&r.ref);if(e)return e;
  const char *header="CLARICE-INSTALLED 1\n";e=io_write(r.ref,header,strlen(header));r.complete=strlen(header);
 }else if(!e){int n=repo_next(&r,line,sizeof line);if(n!=1||strcmp(line,"CLARICE-INSTALLED 1")){
   if((n==0||n==2)&&!strncmp("CLARICE-INSTALLED 1",line,strlen(line))){
    if(append){e=SetEOF(r.ref,0);if(!e)e=SetFPos(r.ref,fsFromStart,0);if(!e)e=io_write(r.ref,"CLARICE-INSTALLED 1\n",sizeof("CLARICE-INSTALLED 1\n")-1);r.complete=sizeof("CLARICE-INSTALLED 1\n")-1;}r.pos=r.end=0;
   }else e=paramErr;
  }
  while(!e&&(n=repo_next(&r,line,sizeof line))==1){RepoRecord rec;e=repo_record(line,&rec,1);if(!e){RepoItem *item=repo_find(list,&rec);if(item){item->installed=rec;item->known=1;}}if(!e&&app_yield())e=userCanceledErr;}
  if(!e&&n<0)e=n;
 }else return e;
 if(!e&&append){e=SetEOF(r.ref,r.complete);if(!e)e=SetFPos(r.ref,fsFromStart,r.complete);if(!e)e=repo_line(append,line,sizeof line);if(!e)e=io_write(r.ref,line,strlen(line));}
 int x=FSClose(r.ref);if(!e)e=x;if(!e&&append)e=update_flush(native);return e;
}
static int repo_marker(const char *text){
 char native[PATH_CAP],canonical[PATH_CAP];int e=checked(REPO_MARKER,native,canonical,0);if(e)return e;
 if(!text){e=cli_native_delete(native);return e==fnfErr?0:e?e:update_flush(native);}
 CInfoPBRec c;e=info(native,&c);if(e==fnfErr)e=cli_native_create(native,'TEXT','CLRC');if(e)return e;
 short ref;e=cli_native_open(native,0,1,&ref);if(e)return e;e=SetEOF(ref,0);if(!e)e=io_write(ref,text,strlen(text));int x=FSClose(ref);if(!e)e=x;return e?e:update_flush(native);
}
#include "repository-recovery.inc"
static int repo_finish(void){
 char text[768],own[256];int e=read_text(REPO_MARKER,text,sizeof text);if(e==fnfErr)return 0;if(e)return e;
 if(strncmp(text,"CLARICE-UPDATE-READY 1\n",23)){errsay("An update is incomplete. Run clarice update -y to resume.\n");return 0;}
 char *row=text+23,*end=strchr(row,'\n');if(!end)return paramErr;*end++=0;RepoRecord core;e=repo_record(row,&core,1);if(e||!core.core)return paramErr;
 char *nl=strchr(end,'\n');if(!nl||nl[1])return paramErr;*nl=0;
 e=update_application_path(own);if(e)return e;uint32_t build;unsigned system;e=update_stamp(own,&build,&system);if(e)return e;
 if(strcmp(end,own)||build!=core.version){errsay("Core update has not relaunched yet; update remains incomplete.\n");return 0;}
 /* A manual relaunch after the helper installed Core but before it removed
    its plan can finish safely too. Validate the exact candidate and retained
    original before deleting only that consumed plan. */
 char planpath[PATH_CAP],canonical[PATH_CAP];e=checked("/etc/clarice/" UPDATE_PLAN_NAME,planpath,canonical,0);if(e)return e;
 short ref;e=cli_native_open(planpath,0,0,&ref);
 if(!e){UpdatePlan p;unsigned char bytes[UPDATE_PLAN_SIZE];long length;e=GetEOF(ref,&length);if(!e&&length!=sizeof bytes)e=paramErr;if(!e)e=io_read(ref,bytes,sizeof bytes);int x=FSClose(ref);if(!e)e=x;if(!e)e=update_plan_decode(bytes,&p);
  if(!e&&(p.build!=build||strcmp(p.target,own)))e=paramErr;
  CInfoPBRec previous;if(!e){int old=info(p.backup,&previous);if(old==fnfErr){RecoverySnapshot snapshot;RecoveryRotation rotation;old=repo_recovery_load(&snapshot,&rotation);if(!old){old=recovery_verify(rotation.path[0],&snapshot);recovery_dispose(&snapshot);}}else if(!old&&((uint32_t)previous.hFileInfo.ioDirID!=p.original_id||previous.hFileInfo.ioFlMdDat!=p.original_modified))old=paramErr;e=old;}
  if(!e)e=update_verify_candidate(own,&p);if(!e)e=repo_recovery_cleanup(&p,own);if(!e)e=cli_native_delete(planpath);if(!e)e=update_flush(planpath);
 }else if(e==fnfErr)e=repo_recovery_cleanup(0,own);if(e)return e;
 e=repo_catalog(0,&core);if(!e)e=repo_marker(0);if(!e)say("Clarice update complete.\n");return e;
}
static int repo_url(const char *source,const char *file,char *url,unsigned cap){size_t n=strrchr(source,'/')-source+1;if(n+strlen(file)>=cap)return bdNamErr;memcpy(url,source,n);strcpy(url+n,file);return 0;}
static int repo_confirm(void){
 if(!cli_isatty(0)||!cli_isatty(1)){errsay("Use -y for noninteractive updates.\n");return -1;}
 for(;;){char answer[32];say("Install these updates? [Y/n]\n");term_present();long n=cli_read(0,answer,sizeof answer);if(n<0)return -1;int yes=update_confirmation(answer,n);if(yes>=0)return yes;}
}
int clarice_main(int argc,char **argv){
 int check=0,yes=0,e=0,changes=0;char source[768]="",index[PATH_CAP]="",payload[PATH_CAP]="",text[1024],own[256];RepoItem *items=0,*core=0;
 if(argc==3&&!strcmp(argv[1],"update")&&!strcmp(argv[2],"--finish")){e=repo_finish();if(e)error("update completion",e);return e?1:0;}
 if(argc==2&&!strcmp(argv[1],"--help"))return help_output(1,
  "Usage:\n"
  "  clarice update [-y] [--check] [--source PATH_OR_HTTP_URL]\n"
  "Update standard tools and Core, then relaunch. Keeps newer tools, unlisted files and preferences.\n"
  "Options:\n"
  "  -y\tAccept without prompting.\n"
  "  --check\tCheck versions without installing.\n"
  "  --source PATH_OR_HTTP_URL\tOverride /etc/clarice/update.conf with an HTTP URL or absolute mounted-file path.\n"
  "Completed tools survive interruption. Rerun to resume. The installed catalog is /etc/clarice/installed.catalog.\n"
  "Private HTTP uses CRC-32 for corruption detection; it provides no authentication.\n");
 if(argc<2||strcmp(argv[1],"update"))return 2;
 for(int i=2;i<argc;i++){if(!strcmp(argv[i],"-y"))yes=1;else if(!strcmp(argv[i],"--check"))check=1;else if(!strcmp(argv[i],"--source")&&i+1<argc&&strlen(argv[i+1])<sizeof source)strcpy(source,argv[++i]);else return 2;}
 if(!source[0]){e=read_text("/etc/clarice/update.conf",source,sizeof source);if(e)goto done;size_t n=strlen(source);while(n&&(source[n-1]=='\n'||source[n-1]=='\r'))source[--n]=0;}
 if(!update_source_valid(source)){e=paramErr;errsay("Use an absolute mounted-file path or numeric private-network HTTP index URL.\n");goto done;}
 e=update_application_path(own);uint32_t current=0;unsigned system=0;if(!e)e=update_stamp(own,&current,&system);if(e)goto done;
 say("Checking repository...\n");e=download(source,index,0);if(e)goto done;e=repo_read_index(index,&items);if(!e)e=repo_catalog(items,0);if(!e)e=repo_catalog_members(&items);if(e)goto done;
 {char root[256];e=resolve("/",root);if(!e)e=recovery_install_path(root,repository_target);if(e)goto done;repository_migrate=strcmp(repository_target,own)!=0;}
 for(RepoItem *i=items;i;i=i->next){RepoRecord *r=&i->wanted;
  /* Old catalogs may still advertise an external updater. Core owns it now. */
  if(!r->core&&!strcmp(r->name,"clarice"))continue;
  if(r->core){core=i;i->needed=r->version>current||repository_migrate;}else{
   char path[PATH_CAP],native[PATH_CAP],canonical[PATH_CAP];CInfoPBRec cat;snprintf(path,sizeof path,"/bin/%s",r->name);e=checked(path,native,canonical,0);if(e)goto done;e=info(native,&cat);
   if(e&&e!=fnfErr)goto done;int exists=!e;e=0;
   if(exists&&(cat.hFileInfo.ioFlAttrib&16)){e=paramErr;goto done;}
   if(i->known&&i->installed.version>r->version){if(!exists){e=fnfErr;goto done;}continue;}
   if(i->known&&i->installed.version==r->version&&(i->installed.crc!=r->crc||i->installed.size!=r->size)){errsay("Repository changed an already published version.\n");e=paramErr;goto done;}
   i->needed=1;
   if(exists&&cat.hFileInfo.ioFlLgLen==r->size){uint32_t hash;
    if(i->known&&i->installed.size==r->size&&i->installed.crc==r->crc)i->needed=0;
    else {e=update_hash_fork(native,0,&hash,r->size);if(e)goto done;if(hash==r->crc)i->needed=0;}
   }
  }
  if(i->needed){snprintf(text,sizeof text,"%s -> build %lu\n",r->name,(unsigned long)r->version);say(text);changes++;}
 }
 if(check)goto done;
 if(changes&&!yes){int answer=repo_confirm();if(answer<0){e=userCanceledErr;goto done;}if(!answer)goto done;}
 if(changes){e=repo_recovery_prepare(items,own,current);if(e)goto done;}
 e=repo_marker("CLARICE-UPDATE-INCOMPLETE 1\n");if(e)goto done;
 if(repository_migrate){e=repo_recovery_root(own,repository_target);if(e)goto done;}
 for(RepoItem *i=items;i;i=i->next){if(i->wanted.core||!strcmp(i->wanted.name,"clarice"))continue;RepoRecord *r=&i->wanted;
  if(i->known&&i->installed.version>r->version)continue;
  if(i->needed){char url[768],native[PATH_CAP],canonical[PATH_CAP];e=repo_url(source,r->file,url,sizeof url);if(e)goto done;say("Installing ");say(r->name);say("...\n");e=download(url,payload,r->size);if(e)goto done;
   e=checked(payload,native,canonical,0);uint32_t crc;if(!e)e=update_hash_fork(native,0,&crc,r->size);if(!e&&crc!=r->crc)e=paramErr;if(e)goto done;
   e=cli_appinstall(payload,r->name,r->version);if(e)goto done;cli_remove(payload);payload[0]=0;
  }
  if(!i->known||i->installed.version!=r->version||i->installed.size!=r->size||i->installed.crc!=r->crc){e=repo_catalog(0,r);if(e)goto done;}
 }
 if(core&&core->needed){
  char row[256];e=repo_line(&core->wanted,row,sizeof row);if(e)goto done;
  snprintf(text,sizeof text,"CLARICE-UPDATE-READY 1\n%s%s\n",row,repository_target);e=repo_marker(text);if(e)goto done;
  memset(&repository_core,0,sizeof repository_core);repository_core.build=core->wanted.version;repository_core.bytes=core->wanted.size;repository_core.system=0x600;snprintf(repository_core.version,sizeof repository_core.version,"build %lu",(unsigned long)core->wanted.version);snprintf(repository_core.crc,sizeof repository_core.crc,"%08lx",(unsigned long)core->wanted.crc);repository_package=core->wanted.file;
  char *args[]={"clarice","update","-y","--source",source};e=core_update(5,args);if(e)e=e==130?userCanceledErr:ioErr;
 }else{RepoRecord r;memset(&r,0,sizeof r);r.core=1;strcpy(r.name,"Clarice");r.version=current;e=core&&core->known&&core->installed.version==current?0:repo_catalog(0,&r);if(!e)e=repo_recovery_cleanup(0,own);if(!e)e=repo_marker(0);if(!e)say("All standard tools are current. Update complete.\n");}
done:
 if(payload[0])cli_remove(payload);if(index[0])cli_remove(index);while(items){RepoItem *next=items->next;cli_free(items);items=next;}
 if(e){error("clarice update",e);errsay("Completed packages are retained. Repeat clarice update -y to resume.\n");}return e==userCanceledErr?130:e?1:0;
}
