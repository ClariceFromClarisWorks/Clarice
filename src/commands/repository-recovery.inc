/* One coherent live-installation snapshot. Update retries reuse it; they never
   assemble a rollback generation from independently dated .old files. */
extern int cli_appcheck(const char *);
static int repo_recovery_validate(const char *native){char path[264];snprintf(path,sizeof path,"native:%s",native);return cli_appcheck(path);}
static int repo_recovery_paths(const char *root,RecoveryRotation *r){
 const char *names[]={RECOVERY_CURRENT,RECOVERY_PENDING,RECOVERY_RETIRED};
 for(int i=0;i<3;i++){if(strlen(root)+strlen(names[i])+13>256)return bdNamErr;strcpy(r->path[i],root);strcat(r->path[i],"etc:clarice:");strcat(r->path[i],names[i]);}return 0;
}
/* Include catalogued managed tools no longer listed by the source. */
static int repo_catalog_members(RepoItem **items){
 RepoReader r;memset(&r,0,sizeof r);int e=open_path(REPO_CATALOG,fsRdPerm,&r.ref);if(e==fnfErr)return 0;if(e)return e;
 char line[256];int n=repo_next(&r,line,sizeof line);if(n!=1||strcmp(line,"CLARICE-INSTALLED 1"))e=paramErr;
 while(!e&&(n=repo_next(&r,line,sizeof line))==1){RepoRecord rec;e=repo_record(line,&rec,1);if(e)break;
  /* Historical catalog rows remain for rollback, but the retired command is
     not a current repository member after the rename. */
  if(!rec.core&&!strcmp(rec.name,"hfsattr"))continue;
  RepoItem *item=repo_find(*items,&rec);if(!item){item=cli_alloc(sizeof *item);if(!item){e=memFullErr;break;}memset(item,0,sizeof *item);item->wanted=rec;item->next=*items;*items=item;}
  item->known=1;item->installed=rec;
 }
 if(!e&&n<0)e=n;int x=FSClose(r.ref);return e?e:x;
}
static int repo_recovery_prepare(RepoItem *items,const char *own,uint32_t build,uint32_t index_size,uint32_t index_crc,int changes){
 char root[256],canonical[PATH_CAP],marker[PATH_CAP];int e=resolve("/",root);if(e)return e;
 RecoveryRotation rotation;e=repo_recovery_paths(root,&rotation);if(e)return e;
 e=checked(REPO_MARKER,marker,canonical,0);CInfoPBRec mark;if(!e)e=info(marker,&mark);
 if(!e){
  char text[768];const char *body;uint32_t size,crc;int bound;
  e=read_text(REPO_MARKER,text,sizeof text);if(e)return e;
  e=repo_marker_parse(text,&size,&crc,&body,&bound);if(e<0)return e;
  if(bound&&(size!=index_size||crc!=index_crc)){errsay("Incomplete update belongs to a different repository index. Resume the original index or use Recover Clarice.\n");return paramErr;}
  RecoverySnapshot snapshot;e=recovery_inventory(rotation.path[0],&snapshot,0);
  if(!e){if(strcmp(snapshot.header.root,root))e=paramErr;else e=recovery_verify(rotation.path[0],&snapshot);
   /* Old markers have no index fingerprint. Permit only the already captured
      membership; an originally absent tool must match its recorded payload. */
   for(RepoItem *i=items;!e&&i;i=i->next){if(i->wanted.core||!strcmp(i->wanted.name,"clarice"))continue;
    RecoveryFile *f=0;for(uint32_t n=0;n<snapshot.header.count;n++)if(!strcmp(snapshot.files[n].name,i->wanted.name))f=&snapshot.files[n];
    if(!f||(!f->id&&(f->data!=i->wanted.size||f->data_crc!=i->wanted.crc)))e=paramErr;
   }
   recovery_dispose(&snapshot);
  }
  if(e)errsay("Incomplete update does not match its verified recovery inventory; use the original repository or Recover Clarice.\n");return e;
 }
 if(e!=fnfErr)return e;if(!changes)return 0;
 /* Refuse to call an altered catalogued collection known-good. Independent
    appinstall changes remain untouched and require explicit reconciliation. */
 for(RepoItem *i=items;i;i=i->next)if(i->known){
  if(i->installed.core){if(i->installed.version!=build)return paramErr;continue;}
  char path[256];if(strlen(root)+strlen(i->wanted.name)+5>256)return bdNamErr;strcpy(path,root);strcat(path,"bin:");strcat(path,i->wanted.name);CInfoPBRec c;e=info(path,&c);if(e)return e;
  uint32_t crc;e=update_hash_fork(path,0,&crc,i->installed.size);if(!e&&crc!=i->installed.crc)e=paramErr;if(e){errsay("Installed tool differs from its catalog; recovery snapshot not replaced.\n");return e;}
 }
 char helper[256];if(strlen(root)+strlen(update_helper_leaf)+13>256)return bdNamErr;strcpy(helper,root);strcat(helper,"etc:clarice:");strcat(helper,update_helper_leaf);e=install_helper(helper);if(e)return e;
 RecoverySnapshot s;memset(&s,0,sizeof s);strcpy(s.header.root,root);strcpy(s.header.origin,own);s.header.build=build;s.header.count=3;
 {char target[256];e=locations_core(root,target);if(e)return e;if(!strcmp(target,own))s.header.format=3;}
 for(RepoItem *i=items;i;i=i->next)if(!i->wanted.core&&strcmp(i->wanted.name,"clarice"))s.header.count++;
 CInfoPBRec original;e=info(own,&original);if(e)return e;s.header.origin_id=original.hFileInfo.ioDirID;s.header.origin_modified=original.hFileInfo.ioFlMdDat;
 if(s.header.count>0x7fffffffUL/sizeof *s.files)return memFullErr;s.files=cli_alloc(s.header.count*sizeof *s.files);if(!s.files)return memFullErr;memset(s.files,0,s.header.count*sizeof *s.files);
 strcpy(s.files[0].name,"@core");strcpy(s.files[1].name,"@helper");strcpy(s.files[2].name,"@catalog");uint32_t n=3;
 for(RepoItem *i=items;i;i=i->next)if(!i->wanted.core&&strcmp(i->wanted.name,"clarice")){strcpy(s.files[n].name,i->wanted.name);s.files[n].data=i->wanted.size;s.files[n].data_crc=i->wanted.crc;n++;}
 say("Preparing previous-installation recovery snapshot...\n");/* Catalog hashes and native fork readback bind the saved bytes. Archived
    optional diagnostics need not be runnable by this Core to be preserved. */
 e=recovery_prepare(&rotation,&s,helper,0);recovery_dispose(&s);return e;
}
static int repo_recovery_load(RecoverySnapshot *s,RecoveryRotation *r){char root[256];int e=resolve("/",root);if(!e)e=repo_recovery_paths(root,r);if(!e){CInfoPBRec directory;e=info(r->path[0],&directory);if(!e){e=recovery_inventory(r->path[0],s,0);if(e==fnfErr||e==dirNFErr)e=paramErr;}}if(!e&&strcmp(root,s->header.root)){recovery_dispose(s);e=paramErr;}return e;}
static int repo_recovery_root(const char *own,char target[256]){
 char root[256];int e=resolve("/",root);if(!e)e=locations_core(root,target);if(e||!strcmp(own,target))return e;
 RecoverySnapshot s;RecoveryRotation r;e=repo_recovery_load(&s,&r);if(e)return e;
 const RecoveryFile *core=0;for(uint32_t i=0;i<s.header.count;i++)if(!strcmp(s.files[i].name,"@core"))core=&s.files[i];
 if(!core)e=paramErr;else{
  CInfoPBRec c;e=info(target,&c);
  if(e==fnfErr){char source[256];e=recovery_member(r.path[0],"@core",source);if(!e){RecoveryFile copy;e=recovery_copy(source,target,"@core",&copy);}if(!e)e=update_flush(target);}
  else if(!e)e=recovery_check_file(target,core,0);
 }
 recovery_dispose(&s);return e;
}
/* Only identities explicitly captured by this managed transaction qualify.
   Legacy .clx/.previous/.cu archives and unrelated historical builds stay put. */
static int repo_recovery_cleanup(const UpdatePlan *plan,const char *running){
 RecoverySnapshot s;RecoveryRotation r;int e=repo_recovery_load(&s,&r);if(e==fnfErr)return 0;if(e)return e;
 e=recovery_verify(r.path[0],&s);if(!e)e=recovery_retire_tool(&s,"hfsattr","hfsattrib",repo_recovery_validate);if(!e)e=recovery_clean_tools(&s);if(e){recovery_dispose(&s);return e;}
 /* An unrecognised legacy copy is not owned by this migration. */
 {char old[256];strcpy(old,s.header.root);strcat(old,"bin:hfsattr");CInfoPBRec c;
  if(!info(old,&c))errsay("Unmanaged /bin/hfsattr preserved; inspect it before removing manually.\n");}
 for(uint32_t i=0;!e&&i<s.header.count;i++){
  RecoveryFile *f=&s.files[i];char path[256];CInfoPBRec c;

  if(!strcmp(f->name,"@helper")&&f->source_id){
   char parent[256],leaf[256];if(strlen(s.header.root)+12>sizeof parent){e=bdNamErr;break;}strcpy(parent,s.header.root);strcat(parent,"etc:clarice");
   for(int n=1;;n++){e=cli_native_nth(parent,n,leaf,&c);if(e==fnfErr){e=0;break;}if(e)break;if((uint32_t)c.hFileInfo.ioDirID!=f->source_id)continue;
    e=recovery_join(parent,leaf,path);if(!e)e=recovery_check_file(path,f,0);if(!e){int busy=1;e=update_open_guard(path,&busy);if(!e&&busy)e=fBsyErr;}if(!e)e=cli_native_delete(path);break;
   }
  }
  if(!strcmp(f->name,"@core")){
   if(plan){e=info(plan->backup,&c);if(e==fnfErr)e=0;else{if(!e&&(uint32_t)c.hFileInfo.ioDirID!=plan->original_id)e=paramErr;if(!e)e=recovery_check_file(plan->backup,f,0);if(!e)e=cli_native_delete(plan->backup);}}
   if(!e&&strcmp(s.header.origin,running)){e=info(s.header.origin,&c);if(e==fnfErr)e=0;else{if(!e&&((uint32_t)c.hFileInfo.ioDirID!=s.header.origin_id||c.hFileInfo.ioFlMdDat!=s.header.origin_modified))e=paramErr;if(!e)e=recovery_check_file(s.header.origin,f,0);if(!e){int busy=1;e=update_open_guard(s.header.origin,&busy);if(!e&&busy)e=fBsyErr;}if(!e)e=cli_native_delete(s.header.origin);}}
  }
 }
 if(!e)e=update_flush(s.header.root);recovery_dispose(&s);return e;
}
