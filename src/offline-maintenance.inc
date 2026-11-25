/* Offline policy uses the sealed collection and existing installed catalog.
 * Per-file replacement remains appinstall's staged, fork-preserving operation.
 * Explicit repair keeps manual .clx recovery copies and never rotates a damaged
 * installation into the known-good snapshot. Repair requires matching Core. */
#include "update/repository-marker.h"
#include "update/catalog-reader.h"
typedef struct {uint32_t version,size,crc;int known,needed;} OfflineTool;
static uint32_t offline_manifest_crc;
/* Reuse the existing marker grammar unchanged. Bind interrupted work to the
 * verified kit, never to an unchecked replacement source. */
static int offline_pending(void){
 char path[256],text[768];strcpy(path,root_path);strcat(path,"etc:clarice:update.incomplete");
 short ref;int e=update_open(path,0,0,&ref);if(e==fnfErr)return 0;if(e)return e;long n;
 e=GetEOF(ref,&n);if(!e&&(n<0||n>=sizeof text))e=paramErr;if(!e)e=recovery_read(ref,text,n);int x=FSClose(ref);if(!e)e=x;if(e)return e;text[n]=0;
 uint32_t size,crc;const char *body;int bound;int kind=repo_marker_parse(text,&size,&crc,&body,&bound);if(kind<0)return kind;
 if(bound&&(size!=RELEASE_MANIFEST_SIZE||crc!=offline_manifest_crc))return paramErr;
 return 0;
}
static long offline_catalog_end;
static int offline_catalog_exists;
static int offline_catalog_read(OfflineTool *tools,uint32_t *core){
 char path[256];strcpy(path,root_path);strcat(path,"etc:clarice:installed.catalog");
 RepoReader r;memset(&r,0,sizeof r);int e=cli_native_open(path,0,0,&r.ref);
 offline_catalog_exists=e!=fnfErr;offline_catalog_end=0;*core=0;if(e==fnfErr)return 0;if(e)return e;
 char line[256];int n=repo_next(&r,line,sizeof line);
 if(n!=1||strcmp(line,"CLARICE-INSTALLED 1")){
  if((n==0||n==2)&&!strncmp("CLARICE-INSTALLED 1",line,strlen(line))){int x=FSClose(r.ref);return x;}
  e=paramErr;
 }
 while(!e&&(n=repo_next(&r,line,sizeof line))==1){RepoRecord rec;e=repo_record(line,&rec,1);if(e)break;
  if(rec.core)*core=rec.version;
  else for(unsigned i=0;i<RELEASE_COUNT;i++)if(!strcmp(rec.name,release_names[i])){tools[i].known=1;tools[i].version=rec.version;tools[i].size=rec.size;tools[i].crc=rec.crc;break;}
  if(app_yield())e=userCanceledErr;
 }
 if(!e&&n<0)e=n;offline_catalog_end=r.complete;int x=FSClose(r.ref);return e?e:x;
}
static int offline_catalog_open(short *ref){
 char path[256];strcpy(path,root_path);strcat(path,"etc:clarice:installed.catalog");int e=0;
 if(!offline_catalog_exists)e=cli_native_create(path,'TEXT','CLRC');if(e)return e;
 unsigned char name[256];pstr(name,path);e=HOpen(0,0,name,fsRdWrPerm,ref);if(e)return e;
 e=SetEOF(*ref,offline_catalog_end);if(!e)e=SetFPos(*ref,fsFromStart,offline_catalog_end);
 if(!e&&!offline_catalog_end)e=recovery_write(*ref,"CLARICE-INSTALLED 1\n",sizeof("CLARICE-INSTALLED 1\n")-1);
 if(e){FSClose(*ref);*ref=-1;}return e;
}
static int offline_catalog_append(short ref,const RepoRecord *r){char line[256];int e=repo_line(r,line,sizeof line);if(!e)e=recovery_write(ref,line,strlen(line));return e?e:update_flush(root_path);}
static int offline_manifest(const char *media,unsigned char *bytes){
 char path[256];int e=recovery_join(media,"Release",path);short ref=-1;if(!e)e=update_open(path,0,0,&ref);long n=0;
 if(!e)e=GetEOF(ref,&n);if(!e&&n!=RELEASE_MANIFEST_SIZE)e=paramErr;if(!e)e=recovery_read(ref,bytes,RELEASE_MANIFEST_SIZE);
 if(ref>=0){int x=FSClose(ref);if(!e)e=x;}if(!e)e=release_manifest_check(bytes,RELEASE_MANIFEST_SIZE);
 if(!e){UpdateCRC32 crc;update_crc32_init(&crc);update_crc32_bytes(&crc,bytes,RELEASE_MANIFEST_SIZE);offline_manifest_crc=update_crc32_sum(&crc);}return e;
}
static int offline_tool_check(const char *path,uint32_t size,uint32_t expected){
 CInfoPBRec c;int e=info(path,&c);if(e)return e;
 if(c.hFileInfo.ioFlAttrib&16||c.hFileInfo.ioFlRLgLen||c.hFileInfo.ioFlFndrInfo.fdType!='CLX1'||c.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC')return paramErr;
 uint32_t crc;e=update_hash_fork(path,0,&crc,size);return e?e:crc==expected?0:paramErr;
}
/* Check every input before replacing any managed file. Unknown or modified
 * names are not ownership evidence; only explicit repair may replace them. */
static int offline_tools_plan(const char *kit,const unsigned char *manifest,OfflineTool *tools,int repair,int *changes){
 uint32_t version=update_be32(manifest+8);*changes=0;
 for(unsigned i=0;i<RELEASE_COUNT;i++){
  const char *name=release_names[i];const unsigned char *row=manifest+56+32*i;
  uint32_t size=update_be32(row+24),crc=update_be32(row+28);char source[256],dest[PATH_CAP],stage[PATH_CAP],backup[PATH_CAP];
  int e=recovery_join(kit,name,source);uint32_t hash;if(!e)e=update_hash_fork(source,0,&hash,size);if(!e&&hash!=crc)e=paramErr;if(e)return e;
  e=command_paths(name,dest,stage,backup);if(e)return e;
  e=offline_tool_check(dest,size,crc);tools[i].needed=0;if(!e)continue;
  if(e!=fnfErr&&e!=paramErr)return e;
  if(e==fnfErr){tools[i].needed=1;(*changes)++;continue;}
  if(!repair){
   if(!tools[i].known){errsay("Unmanaged standard tool kept: ");errsay(name);errsay(". Choose explicit Repair to replace it.\n");return paramErr;}
   e=offline_tool_check(dest,tools[i].size,tools[i].crc);if(e){errsay("Changed standard tool kept: ");errsay(name);errsay(". Choose explicit Repair to replace it.\n");return e;}
   if(tools[i].version>version){e=command_verify_native(dest);if(e)return e;continue;}
   if(tools[i].version==version){errsay("Kit differs from the catalog for the same version.\n");return paramErr;}
  }
  tools[i].needed=1;(*changes)++;
  if(app_yield())return userCanceledErr;
 }return 0;
}
static int offline_tools_install(const char *kit,const unsigned char *manifest,OfflineTool *tools,int repair){
 short ref=-1;int e=offline_catalog_open(&ref);if(e)return e;uint32_t build=update_be32(manifest+8);
 for(unsigned i=0;i<RELEASE_COUNT&&!e;i++){
  app_install_progress(release_names[i],i+2,RELEASE_COUNT+4);
  char source[264],native[256],dest[PATH_CAP],stage[PATH_CAP],backup[PATH_CAP];const unsigned char *row=manifest+56+i*32;
  e=recovery_join(kit,release_names[i],native);if(e)break;snprintf(source,sizeof source,"native:%s",native);
  if(tools[i].needed)e=command_install_resume(source,release_names[i],!repair);
  if(e)break;e=command_paths(release_names[i],dest,stage,backup);if(e)break;
  /* Explicit repair retains the original (including damaged Finder codes) in
   * its manual backup. The replacement must be identifiable as a CLX tool. */
  if(repair&&tools[i].needed){CInfoPBRec c;e=info(dest,&c);
   if(!e&&(c.hFileInfo.ioFlFndrInfo.fdType!='CLX1'||c.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC')){
    c.hFileInfo.ioFlFndrInfo.fdType='CLX1';c.hFileInfo.ioFlFndrInfo.fdCreator='CLRC';e=cli_native_set(dest,&c);if(!e)e=update_flush(dest);
   }if(e)break;
  }
  if(!repair&&tools[i].known&&tools[i].version>build)e=offline_tool_check(dest,tools[i].size,tools[i].crc);
  else{
   RepoRecord record;memset(&record,0,sizeof record);strcpy(record.name,release_names[i]);record.version=build;record.size=update_be32(row+24);record.crc=update_be32(row+28);
   e=offline_tool_check(dest,record.size,record.crc);
   if(!e&&(!tools[i].known||tools[i].version!=build||tools[i].size!=record.size||tools[i].crc!=record.crc))e=offline_catalog_append(ref,&record);
  }
  if(!e&&app_yield())e=userCanceledErr;
 }
 int x=FSClose(ref);return e?e:x;
}
