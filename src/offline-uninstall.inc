/* Installer-only removal. The catalog is kept until the last data deletion so
 * an interrupted removal remains identifiable. No Unix pathname resolution is
 * used: enumeration and deletion use the volume, parent ID and literal name. */
static int uninstall_path(const InstallLocations *p,const char *leaf,char out[256]){
 if(strlen(p->data)+strlen(leaf)>255)return bdNamErr;
 strcpy(out,p->data);strcat(out,leaf);if(!*leaf)out[strlen(out)-1]=0;return 0;
}
static int uninstall_catalog(const char *path,uint32_t *build){
 CInfoPBRec c;int e=update_info(path,&c);if(e)return e;
 if(c.hFileInfo.ioFlAttrib&16||c.hFileInfo.ioFlRLgLen||c.hFileInfo.ioFlFndrInfo.fdType!='TEXT'||c.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC')return paramErr;
 RepoReader r;memset(&r,0,sizeof r);e=update_open(path,0,0,&r.ref);if(e)return e;
 char line[256];int n=repo_next(&r,line,sizeof line);*build=0;
 if(n!=1||strcmp(line,"CLARICE-INSTALLED 1"))e=paramErr;
 while(!e&&(n=repo_next(&r,line,sizeof line))==1){RepoRecord rec;e=repo_record(line,&rec,1);if(!e&&rec.core)*build=rec.version;if(app_yield())e=userCanceledErr;}
 if(!e&&(n||!*build))e=n<0?n:paramErr;int x=FSClose(r.ref);return e?e:x;
}
/* Read-only discovery. Missing/invalid catalogs do not authorize recursive
 * deletion. An empty final scaffold is handled separately for retry. */
static int uninstall_probe(InstallLocations *p,uint32_t *build,int *present){
 int recorded=1,e=locations_read(p);if(e==fnfErr){recorded=0;e=locations_default(p);}if(e)return e;
 if(!locations_valid(p))return paramErr;
 char path[256];CInfoPBRec c;e=uninstall_path(p,"etc:clarice:installed.catalog",path);if(e)return e;
 e=uninstall_catalog(path,build);if(e)return e;
 e=update_info(p->core,&c);
 if(!recorded&&e==fnfErr){char old[256];strcpy(old,p->data);old[strlen(old)-13]=0;strcat(old,"Clarice");e=update_info(old,&c);if(!e)strcpy(p->core,old);}
 *present=e!=fnfErr;if(e==fnfErr)return 0;if(e)return e;
 if(c.hFileInfo.ioFlAttrib&16||c.hFileInfo.ioFlFndrInfo.fdType!='APPL'||c.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC')return paramErr;
 uint32_t actual;unsigned system;e=update_stamp(p->core,&actual,&system);if(e)return e;
 if(actual!=*build)return paramErr;
 int busy;e=update_open_guard(p->core,&busy);return e?e:busy?fBsyErr:0;
}
/* One stack entry per possible HFS component in a 255-byte installation path.
 * The tree itself may have deeper native paths: reject safely at this bound,
 * never truncate a name. No invocation or permanent heap allocation. */
static int uninstall_tree(short volume,long root,long etc,long settings,long catalog,int dry){
 struct {long id;short index;} stack[128];unsigned depth=0,removed=0;
 stack[0].id=root;stack[0].index=1;
 for(;;){
  if(app_yield())return userCanceledErr;
  unsigned char name[256];CInfoPBRec c;memset(&c,0,sizeof c);
  c.hFileInfo.ioNamePtr=name;c.hFileInfo.ioVRefNum=volume;c.hFileInfo.ioDirID=stack[depth].id;c.hFileInfo.ioFDirIndex=stack[depth].index;
  int e=PBGetCatInfoSync(&c);
  if(e==fnfErr){
   if(!depth)return 0;
   long id=stack[depth--].id;
   if(id==etc||id==settings){stack[depth].index++;continue;}
   memset(&c,0,sizeof c);c.dirInfo.ioNamePtr=name;c.dirInfo.ioVRefNum=volume;c.dirInfo.ioDrDirID=id;c.dirInfo.ioFDirIndex=-1;
   e=PBGetCatInfoSync(&c);if(e)return e;
   if(c.dirInfo.ioDrParID!=stack[depth].id)return paramErr;
  }else if(e)return e;
  else if(c.hFileInfo.ioFlAttrib&16){
   long id=c.dirInfo.ioDrDirID;if(id<=2||depth+1>=128)return paramErr;
   if(dry&&id!=etc&&id!=settings)return paramErr;
   stack[++depth].id=id;stack[depth].index=1;continue;
  }else if((long)c.hFileInfo.ioDirID==catalog){stack[depth].index++;continue;}
  else if(dry)return paramErr;
  /* File Manager interprets neither '.' nor '/' specially in this leaf. */
  e=HDelete(volume,stack[depth].id,name);if(e)return e;
  if(!(++removed&15))app_install_progress("Removing Clarice Data (including /home)",removed,removed+1);
 }
}
static int uninstall_dirs(const InstallLocations *p,short *volume,long ids[3],char catalog[256],long *catid){
 char path[256];const char *leaf[]={"","etc","etc:clarice"};int e=update_volume(p->data,volume);if(e)return e;
 for(int i=0;i<3;i++){
  e=uninstall_path(p,leaf[i],path);if(e)return e;CInfoPBRec c;e=update_info(path,&c);
  if(e==fnfErr||e==dirNFErr){ids[i]=0;continue;}if(e)return e;
  if(!(c.hFileInfo.ioFlAttrib&16)||c.dirInfo.ioDrDirID<=2)return paramErr;ids[i]=c.dirInfo.ioDrDirID;
 }
 e=uninstall_path(p,"etc:clarice:installed.catalog",catalog);if(e)return e;
 CInfoPBRec c;e=update_info(catalog,&c);*catid=0;if(!e)*catid=c.hFileInfo.ioDirID;
 return e==fnfErr||e==dirNFErr?0:e;
}
int shell_uninstall_locations(char *core,char *data){
 InstallLocations p;uint32_t build;int present;int e=uninstall_probe(&p,&build,&present);
 if(e==fnfErr||e==dirNFErr){
  /* After deleting the catalog, only empty protected directories may remain.
   * The immutable location record still identifies them; never adopt files. */
  e=locations_read(&p);if(e)return e;CInfoPBRec c;e=update_info(p.core,&c);if(e!=fnfErr)return e?e:paramErr;
  short v;long ids[3],cat;char path[256];e=uninstall_dirs(&p,&v,ids,path,&cat);if(e)return e;
  if(cat)return paramErr;if(ids[0])e=uninstall_tree(v,ids[0],ids[1],ids[2],0,1);
 }
 if(e)return e;strcpy(core,p.core);strcpy(data,p.data);return 0;
}
int shell_uninstall(const char *core,const char *data,int all){
 InstallLocations p;int e=shell_uninstall_locations(p.core,p.data);if(e)return e;
 if(strcmp(core,p.core)||strcmp(data,p.data))return paramErr;
 char path[256],own[256];e=update_application_path(own);if(e)return e;
 /* Never remove the running installer or pending update/recovery state. */
 unsigned char a[256],b[256];update_pstr(a,own);update_pstr(b,p.core);
 if(EqualString(a,b,0,1))return fBsyErr;
 if(all){
  /* A selected Data folder must never contain the active System Folder or
   * this installer, even after a user has rearranged folders in Finder. */
  char system[256];e=locations_system(system);if(e)return e;
  const char *protected[]={own,system};size_t n=strlen(p.data);
  for(unsigned i=0;i<2;i++)if(strlen(protected[i])>=n){memcpy(a+1,protected[i],n);a[0]=n;update_pstr(b,p.data);if(EqualString(a,b,0,1))return fBsyErr;}
 }
 const char *pending[]={"etc:clarice:update.incomplete","etc:clarice:" UPDATE_PLAN_NAME};
 CInfoPBRec c;for(unsigned i=0;i<2;i++){e=uninstall_path(&p,pending[i],path);if(e)return e;e=update_info(path,&c);if(!e)return fBsyErr;if(e!=fnfErr&&e!=dirNFErr)return e;}
 app_install_progress("Removing the identified Clarice application",0,2);
 if(app_yield())return userCanceledErr;
 e=locations_write(&p);if(e)return e;
 e=update_info(p.core,&c);if(!e)e=recovery_delete(p.core);else if(e==fnfErr)e=0;
 if(!e)e=update_flush(p.core);if(e||!all)return e;
 short volume;long ids[3],catid;char catalog[256];e=uninstall_dirs(&p,&volume,ids,catalog,&catid);if(e)return e;
 if(ids[0])e=uninstall_tree(volume,ids[0],ids[1],ids[2],catid,0);if(e)return e;
 /* Preserve catalog authority until every other entry is gone. If interrupted
  * here, discovery accepts only the empty scaffold, not a substitute tree. */
 if(catid){e=recovery_delete(catalog);if(e)return e;}
 const char *last[]={"etc:clarice","etc",""};
 for(unsigned i=0;i<3;i++)if(ids[2-i]){e=uninstall_path(&p,last[i],path);if(!e)e=recovery_delete(path);if(e)return e;}
 e=locations_record_path(path);if(!e){InstallLocations saved;int r=locations_read(&saved);if(!r){if(strcmp(saved.core,p.core)||strcmp(saved.data,p.data))return paramErr;e=recovery_delete(path);}else if(r!=fnfErr)e=r;}
 if(!e)e=update_flush(p.core);return e;
}
