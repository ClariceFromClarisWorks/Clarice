/* Private-source single-command updater. Networking and verification are loaded
 * only for this invocation; independent helper performs the post-exit commit. */
#include "mac-files.h"
#include <OSUtils.h>
#include <stdint.h>
#include "../update/identity.h"
#include "../update/manifest.h"
#include "../update/confirmation.h"
#include "../update/extract.h"
static void verification_failure(const char *phase,uint32_t want,uint32_t have){char text[128];snprintf(text,sizeof text,"%s mismatch: expected %08lx, read %08lx.\n",phase,(unsigned long)want,(unsigned long)have);errsay(text);}
#define UPDATE_VERIFY_REPORT(phase,want,have) verification_failure(phase,want,have)
#define UPDATE_VERIFY_POLL() (app_yield()?userCanceledErr:0)
#include "../update/verify.h"
#include "update-helper-image.h"
extern int open_path(const char *,short,short *);
extern unsigned long NetParseIP(const char *);
extern int wget_fetch(const char *,const char *,unsigned long);
extern int cli_isatty(int),cli_temp_create(char *,unsigned long),cli_remove(const char *);
extern long cli_read(int,void *,unsigned long);
static int io_read(short ref,void *p,long n){long want=n;int e=FSRead(ref,&n,p);return e?e:n==want?0:ioErr;}
static int io_write(short ref,const void *p,long n){long want=n;int e=FSWrite(ref,&n,p);return e?e:n==want?0:ioErr;}
static int absent(const char *path){CInfoPBRec p;int e=info(path,&p);return e==fnfErr?0:e?e:dupFNErr;}
static int read_text(const char *path,char *out,unsigned cap){short ref=-1;int e=open_path(path,fsRdPerm,&ref);if(e)return e;long size;e=GetEOF(ref,&size);if(!e&&(size<0||size>=(long)cap))e=paramErr;if(!e){e=io_read(ref,out,size);out[size]=0;if(memchr(out,0,size))e=paramErr;}int x=FSClose(ref);return e?e:x;}
#include "../update/source.h"
static int download(const char *url,char path[PATH_CAP],unsigned long maximum){
 if(cli_temp_create(path,PATH_CAP))return ioErr;
 if(url[0]=='/')return update_copy_source(url,path,maximum);
 if(cli_remove(path))return ioErr;int r=wget_fetch(url,path,maximum);
 return r==130?userCanceledErr:r?ioErr:0;
}
typedef struct {short in,out;const unsigned char *memory;uint32_t length,offset;const char *dest;UpdateCRC32 hash,package_crc;uint32_t hashes[2],fork_offset,resource_size;int fork;} Extract;
static int ex_read(void *c,void *p,unsigned n){Extract *s=c;if(n>s->length-s->offset)return eofErr;int e=0;if(s->memory)memcpy(p,s->memory+s->offset,n);else e=io_read(s->in,p,n);if(!e){s->offset+=n;update_crc32_bytes(&s->package_crc,p,n);if(s->out>=0){if(s->fork&&!s->fork_offset&&(n<16||update_resource_layout(p,s->resource_size)))return paramErr;update_fork_crc_bytes(&s->hash,s->fork,s->fork_offset,p,n);s->fork_offset+=n;}}return e;}
/* Consume the header and alignment padding as well as both forks, so the
 * manifest CRC checks the entire downloaded package during the existing copy. */
static int ex_seek(void *c,uint32_t n){Extract *s=c;if(n>s->length||n<s->offset)return paramErr;unsigned char padding[128];while(s->offset<n){unsigned count=n-s->offset;if(count>sizeof padding)count=sizeof padding;int e=ex_read(s,padding,count);if(e)return e;}return 0;}
static int ex_begin(void *c,int fork){Extract *s=c;s->fork=fork;s->fork_offset=0;update_crc32_init(&s->hash);return cli_native_open(s->dest,fork,1,&s->out);}
static int ex_write(void *c,const void *p,unsigned n){return io_write(((Extract*)c)->out,p,n);}
static int ex_end(void *c){Extract *s=c;int e=FSClose(s->out);s->out=-1;if(!e)s->hashes[s->fork]=update_crc32_sum(&s->hash);return e;}
static int ex_poll(void *c){(void)c;return app_yield()?userCanceledErr:0;}
static int extract(Extract *s,const UpdatePackage *p){s->resource_size=p->resource_size;update_crc32_init(&s->package_crc);s->offset=0;int e=s->memory?0:SetFPos(s->in,fsFromStart,0);if(e)return e;UpdateExtractOps o={s,ex_seek,ex_read,ex_begin,ex_write,ex_end,ex_poll};e=update_extract(&o,p);return e?e:ex_seek(s,s->length);}
static int install_helper(const char *path){
 UpdatePackage p;int e=update_package_kind(update_helper_image,sizeof update_helper_image,&p,"CLUP");if(e)return e;
 CInfoPBRec cat;e=info(path,&cat);
 if(!e){
  if(cat.hFileInfo.ioFlFndrInfo.fdType!='APPL'||cat.hFileInfo.ioFlFndrInfo.fdCreator!='CLUP')return paramErr;
  for(int fork=0;fork<2;fork++){uint32_t want,have;UpdateCRC32 ctx;uint32_t n=fork?p.resource_size:p.data_size;update_crc32_init(&ctx);update_fork_crc_bytes(&ctx,fork,0,update_helper_image+(fork?p.resource_offset:128),n);want=update_crc32_sum(&ctx);e=update_hash_fork(path,fork,&have,n);if(e||have!=want)return e?e:paramErr;}
  return 0;
 }
 if(e!=fnfErr)return e;
 e=cli_native_create(path,'APPL','CLUP');if(e)return e;Extract s={-1,-1,update_helper_image,sizeof update_helper_image,0,path};e=extract(&s,&p);
 if(!e)e=info(path,&cat);if(!e){cat.hFileInfo.ioFlFndrInfo.fdFlags=p.flags;cat.hFileInfo.ioFlCrDat=p.created;cat.hFileInfo.ioFlMdDat=p.modified;e=cli_native_set(path,&cat);}if(!e)e=update_flush(path);
 if(!e){for(int fork=0;fork<2;fork++){uint32_t hash;e=update_hash_fork(path,fork,&hash,fork?p.resource_size:p.data_size);if(e||hash!=s.hashes[fork]){if(!e)e=paramErr;break;}}}
 if(e){int x=cli_native_delete(path);if(x)error("retained incomplete updater helper",x);}return e;
}
static UpdateManifest repository_core;
static const char *repository_package;
static int core_update(int argc,char **argv){
 int check=0,assume_yes=0,e=0,staged=0,planned=0,handed=0;short in=-1;
 char source[768]="",manifest_file[PATH_CAP]="",package_file[PATH_CAP]="",text[1024],own[256],helper[PATH_CAP],plan_path[PATH_CAP],canonical[PATH_CAP];
 UpdateManifest m;UpdatePlan plan;memset(&plan,0,sizeof plan);
 if(argc==2&&!strcmp(argv[1],"--help")){say("usage: clarice update [-y] [--check] [--source\nhttp://IPv4:port/update.txt]\nUses /etc/clarice/update.conf (one source path or URL). HTTP or absolute mounted-file path.\nChecks for a newer 68000-compatible application, verifies both forks,\nasks before replacing, retains the previous version and relaunches\nClarice automatically.\n-y accepts without prompting; verification and recovery are unchanged.\n--check downloads only the source index and makes no application\nchanges.\n");return 0;}
 if(argc<2||strcmp(argv[1],"update")){errsay("usage: clarice update [-y] [--check] [--source URL]\n");return 2;}
 for(int i=2;i<argc;i++){if(!strcmp(argv[i],"-y"))assume_yes=1;else if(!strcmp(argv[i],"--check")&&!check)check=1;else if(!strcmp(argv[i],"--source")&&i+1<argc&&!source[0]){if(strlen(argv[++i])>=sizeof source)return 2;strcpy(source,argv[i]);}else return 2;}
 if(!source[0]){e=read_text("/etc/clarice/update.conf",source,sizeof source);if(e){errsay("Configure /etc/clarice/update.conf with the private update URL, or use --source URL.\n");goto done;}size_t n=strlen(source);while(n&&(source[n-1]=='\n'||source[n-1]=='\r'))source[--n]=0;}
 if(!update_source_valid(source)){errsay("Use an absolute mounted-file path or numeric private-network HTTP index URL.\n");e=paramErr;goto done;}
 e=update_application_path(own);if(e)goto done;uint32_t current=0;unsigned system=0;e=update_stamp(own,&current,&system);if(e){errsay("Running application has no supported update identity. Install an update-capable bootstrap build first.\n");goto done;}
 m=repository_core;
 SysEnvRec env;e=SysEnvirons(1,&env);if(e)goto done;if(m.system>(unsigned short)env.systemVersion){errsay("Update requires a newer Macintosh system.\n");e=paramErr;goto done;}
 snprintf(text,sizeof text,"Installed build %lu; source %s (build %lu).\n",(unsigned long)current,m.version,(unsigned long)m.build);say(text);
 if(m.build<=current){say("No newer compatible update; no downgrade performed.\n");goto done;}if(check)goto done;
 if(!assume_yes&&(!cli_isatty(0)||!cli_isatty(1))){errsay("Updates require an interactive terminal for confirmation, or -y.\n");e=paramErr;goto done;}
 snprintf(text,sizeof text,"/etc/clarice/%s",update_helper_leaf);e=checked(text,helper,canonical,0);if(e)goto done;e=checked("/etc/clarice/" UPDATE_PLAN_NAME,plan_path,canonical,0);if(e)goto done;e=absent(plan_path);if(e==dupFNErr){
  unsigned char saved[UPDATE_PLAN_SIZE];short ref;e=cli_native_open(plan_path,0,0,&ref);
  if(!e){long length;e=GetEOF(ref,&length);if(!e&&length!=sizeof saved)e=paramErr;if(!e)e=io_read(ref,saved,sizeof saved);int x=FSClose(ref);if(!e)e=x;}
  if(!e)e=update_plan_decode(saved,&plan);
  if(!e&&(strcmp(plan.target,own)||plan.build!=m.build))e=paramErr;
  if(!e)e=install_helper(helper);
  if(!e){say("Resuming the confirmed Core installation.\n");term_present();e=app_update_handoff(helper);if(!e)handed=1;}
  goto done;
 }if(e)goto done;
 strcpy(plan.target,own);CInfoPBRec original;e=info(own,&original);if(e)goto done;if(original.hFileInfo.ioFlAttrib&17){e=permErr;goto done;}
 plan.original_id=original.hFileInfo.ioDirID;plan.original_modified=original.hFileInfo.ioFlMdDat;plan.build=m.build;
 const char *leaf=strrchr(own,':');size_t parent=leaf-own+1;if(parent+29>255){e=bdNamErr;goto done;}
 memcpy(plan.stage,own,parent);snprintf(plan.stage+parent,sizeof plan.stage-parent,".cu-%08lx-%08lx.new",(unsigned long)plan.original_id,(unsigned long)m.build);strcpy(plan.backup,plan.stage);strcpy(plan.backup+strlen(plan.backup)-3,"old");
 e=absent(plan.stage);if(e==dupFNErr){CInfoPBRec stale;e=info(plan.stage,&stale);if(!e){char retained[256];memcpy(retained,own,parent);snprintf(retained+parent,sizeof retained-parent,".cu-orphan-%08lx",(unsigned long)stale.hFileInfo.ioDirID);e=absent(retained);if(!e)e=cli_native_rename(plan.stage,retained);if(!e)e=update_flush(retained);}}if(e||(e=absent(plan.backup)))goto done;
 char url[768];strcpy(url,source);char *last=strrchr(url,'/');if((size_t)(last-url)+1+strlen(repository_package)>=sizeof url){e=bdNamErr;goto done;}strcpy(last+1,repository_package);
 say("Downloading application package...\n");e=download(url,package_file,m.bytes);if(e)goto done;e=open_path(package_file,fsRdPerm,&in);if(e)goto done;long size;e=GetEOF(in,&size);if(e||size<0||(uint32_t)size!=m.bytes){if(!e)e=paramErr;goto done;}
 say("Verifying package...\n");unsigned char header[128];e=SetFPos(in,fsFromStart,0);if(!e)e=io_read(in,header,128);UpdatePackage pkg;if(!e)e=update_package(header,m.bytes,&pkg);if(e)goto done;
 say("Staging application forks...\n");unsigned long stage_start=TickCount();e=cli_native_create(plan.stage,'APPL','CLRC');if(e)goto done;staged=1;Extract ex={in,-1,0,m.bytes,0,plan.stage};e=extract(&ex,&pkg);if(e)goto done;char package_crc[9];snprintf(package_crc,sizeof package_crc,"%08lx",(unsigned long)update_crc32_sum(&ex.package_crc));if(strcmp(package_crc,m.crc)){e=paramErr;errsay("Package CRC-32 mismatch.\n");goto done;}e=FSClose(in);in=-1;if(e)goto done;
 CInfoPBRec cat;e=info(plan.stage,&cat);if(e)goto done;
 /* New resource fork/code comes from the package. Keep existing Finder placement
    and creation date, including its existing bundle flag. */
 cat.hFileInfo.ioFlFndrInfo=original.hFileInfo.ioFlFndrInfo;
 cat.hFileInfo.ioFlXFndrInfo=original.hFileInfo.ioFlXFndrInfo;cat.hFileInfo.ioFlCrDat=original.hFileInfo.ioFlCrDat;cat.hFileInfo.ioFlMdDat=pkg.modified;e=cli_native_set(plan.stage,&cat);if(e)goto done;
 plan.data_size=pkg.data_size;plan.resource_size=pkg.resource_size;plan.created=cat.hFileInfo.ioFlCrDat;plan.modified=cat.hFileInfo.ioFlMdDat;memcpy(plan.metadata,&cat.hFileInfo.ioFlFndrInfo,16);memcpy(plan.extra,&cat.hFileInfo.ioFlXFndrInfo,16);
 plan.data_crc=ex.hashes[0];plan.resource_crc=ex.hashes[1];snprintf(text,sizeof text,"Staging: %lu ticks.\n",TickCount()-stage_start);say(text);say("Checking staged application...\n");unsigned long verify_start=TickCount();e=update_verify_candidate(plan.stage,&plan);snprintf(text,sizeof text,"Staged verification: %lu ticks.\n",TickCount()-verify_start);say(text);if(e)goto done;
 say("Preparing independent updater...\n");e=install_helper(helper);if(e)goto done;if((e=update_flush(plan.stage)))goto done;
 snprintf(text,sizeof text,"Verified %s.\n",m.version);say(text);
 say("Only Clarice will close, install and relaunch.\nThis may take a minute; do not reset or switch off the Mac.\nThe previous application is retained for recovery.\n");
 if(!assume_yes){
  int accept=-1;
  while(accept<0){errsay("Install update and relaunch Clarice? [Y/n]\n");term_present();
   char answer[32];long got=cli_read(0,answer,sizeof answer);
   if(got<0){e=userCanceledErr;goto done;}accept=update_confirmation(answer,got);
   if(accept<0)errsay("Please enter y or n (Enter accepts).\n");
  }
  if(!accept){say("Update cancelled; installed application unchanged.\n");goto done;}
 }

 unsigned char encoded[UPDATE_PLAN_SIZE];e=update_plan_encode(&plan,encoded);if(e)goto done;e=cli_native_create(plan_path,'DATA','CLUP');if(e)goto done;planned=1;short out;e=cli_native_open(plan_path,0,1,&out);if(!e){e=io_write(out,encoded,sizeof encoded);int x=FSClose(out);if(!e)e=x;}if(!e)e=update_flush(plan_path);if(e)goto done;
 say("Closing Clarice now. Please wait for automatic relaunch.\n");term_present();e=app_update_handoff(helper);if(!e)handed=1;
done:
 if(in>=0)FSClose(in);
 if(!handed){if(planned){int x=cli_native_delete(plan_path);if(x)error("retained update plan",x);}if(staged){
#ifdef UPDATE_DIAGNOSTIC_RETAIN_STAGE
 if(e){update_flush(plan.stage);errsay("Diagnostic retained stage: ");errsay(plan.stage);errsay("\n");staged=0;}
#endif
 int x=staged?cli_native_delete(plan.stage):0;if(x)error("retained staged application",x);}}
 if(manifest_file[0])cli_remove(manifest_file);if(package_file[0])cli_remove(package_file);
 if(e)error("clarice update",e);return e==userCanceledErr?130:e?1:0;
}

#include "repository-client.inc"
