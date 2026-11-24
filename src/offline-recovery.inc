/* Installer-only snapshot preparation. Existing catalog is required for an
 * upgrade: an unknown legacy mixture must not be labelled known-good. */
#include "update-helper-stamp.h"
#include "update/recovery-native.h"
#include "update/release-manifest.h"
/* Same complete identity checks as the online helper cache: native type,
 * creator, exact lengths and both fork checksums from the sealed kit. */
static int offline_helper_check(const char *path){
 CInfoPBRec c;int e=info(path,&c);if(e)return e;
 if(c.hFileInfo.ioFlAttrib&17||c.hFileInfo.ioFlFndrInfo.fdType!='APPL'||c.hFileInfo.ioFlFndrInfo.fdCreator!='CLUP')return paramErr;
 uint32_t crc;e=update_hash_fork(path,0,&crc,offline_helper_data);if(!e&&crc!=offline_helper_data_crc)e=paramErr;
 if(!e)e=update_hash_fork(path,1,&crc,offline_helper_resource);if(!e&&crc!=offline_helper_resource_crc)e=paramErr;return e;
}
static int offline_recovery(const char *origin,const char *media,uint32_t build){
 RecoveryRotation rotation;const char *names[]={RECOVERY_CURRENT,RECOVERY_PENDING,RECOVERY_RETIRED};
 if(strlen(root_path)+12+strlen(RECOVERY_PENDING)+1>256)return bdNamErr;
 for(int i=0;i<3;i++){strcpy(rotation.path[i],root_path);strcat(rotation.path[i],"etc:clarice:");strcat(rotation.path[i],names[i]);}
 char marker[256];strcpy(marker,root_path);strcat(marker,"etc:clarice:update.incomplete");CInfoPBRec marked;int pending=info(marker,&marked);
 if(!pending){RecoverySnapshot previous;int x=recovery_inventory(rotation.path[0],&previous,0);if(!x){x=strcmp(previous.header.root,root_path)?paramErr:recovery_verify(rotation.path[0],&previous);recovery_dispose(&previous);}return x;}
 if(pending!=fnfErr)return pending;
 char helper_source[256],helper[256];int e=recovery_join(media,RECOVERY_HELPER,helper_source);if(e)return e;
 uint32_t crc;e=offline_helper_check(helper_source);if(e)return e;
 if(strlen(root_path)+strlen(offline_helper_leaf)+13>256)return bdNamErr;strcpy(helper,root_path);strcat(helper,"etc:clarice:");strcat(helper,offline_helper_leaf);
 CInfoPBRec cat;e=info(helper,&cat);if(e==fnfErr){RecoveryFile copied;e=recovery_copy(helper_source,helper,"@helper",&copied);}if(!e)e=offline_helper_check(helper);if(e)return e;
 RecoverySnapshot s;memset(&s,0,sizeof s);strcpy(s.header.root,root_path);strcpy(s.header.origin,origin);s.header.build=build;
 e=info(origin,&cat);if(e)return e;s.header.origin_id=cat.hFileInfo.ioDirID;s.header.origin_modified=cat.hFileInfo.ioFlMdDat;
 /* Catalog names are deduplicated while reading; grow only if necessary. */
 unsigned cap=RELEASE_COUNT+5;s.files=(RecoveryFile*)NewPtr(cap*sizeof *s.files);if(!s.files)return memFullErr;memset(s.files,0,cap*sizeof *s.files);s.header.count=3;
 strcpy(s.files[0].name,"@core");strcpy(s.files[1].name,"@helper");strcpy(s.files[2].name,"@catalog");
 char catalog[256];strcpy(catalog,root_path);strcat(catalog,"etc:clarice:installed.catalog");short ref=-1;e=update_open(catalog,0,0,&ref);
 char buffer[512],line[256];unsigned used=0;int first=1,core=0;
 while(!e){long n=sizeof buffer;e=FSRead(ref,&n,buffer);int eof=e==eofErr;if(eof)e=0;
  for(long j=0;!e&&j<n;j++){
   if(buffer[j]!='\n'){if(!buffer[j]||used+1>=sizeof line){e=paramErr;break;}line[used++]=buffer[j];continue;}
   line[used]=0;used=0;if(first){first=0;if(strcmp(line,"CLARICE-INSTALLED 1"))e=paramErr;continue;}
   RepoRecord record;e=repo_record(line,&record,1);if(e)break;if(record.core){core=record.version==build?1:-1;continue;}
   if(!strcmp(record.name,"hfsattr")){char old[256];strcpy(old,root_path);strcat(old,"bin:hfsattr");CInfoPBRec present;int found=info(old,&present);if(found==fnfErr)continue;if(found){e=found;break;}}
   unsigned i=3;for(;i<s.header.count;i++)if(!strcmp(s.files[i].name,record.name))break;
   if(i==cap){unsigned next=cap*2;if(next<cap||next>0x7fffffffUL/sizeof *s.files){e=memFullErr;break;}RecoveryFile *p=(RecoveryFile*)NewPtr(next*sizeof *p);if(!p){e=memFullErr;break;}memcpy(p,s.files,cap*sizeof *p);memset(p+cap,0,(next-cap)*sizeof *p);DisposePtr((Ptr)s.files);s.files=p;cap=next;}
   if(i==s.header.count)s.header.count++;strcpy(s.files[i].name,record.name);s.files[i].data=record.size;s.files[i].data_crc=record.crc;
  }
  if(eof||!n)break;
 }
 if(ref>=0){int x=FSClose(ref);if(!e)e=x;}if(!e&&core!=1)e=paramErr;
 for(unsigned i=3;!e&&i<s.header.count;i++){
  char path[256];if(strlen(root_path)+strlen(s.files[i].name)+5>256){e=bdNamErr;break;}strcpy(path,root_path);strcat(path,"bin:");strcat(path,s.files[i].name);
  e=update_hash_fork(path,0,&crc,s.files[i].data);if(!e&&crc!=s.files[i].data_crc)e=paramErr;
 }
 unsigned char manifest[RELEASE_MANIFEST_SIZE];char release[256];short release_ref=-1;
 if(!e)e=recovery_join(media,"Release",release);if(!e)e=update_open(release,0,0,&release_ref);
 if(!e)e=recovery_read(release_ref,manifest,sizeof manifest);if(release_ref>=0){int x=FSClose(release_ref);if(!e)e=x;}if(!e)e=release_manifest_check(manifest,sizeof manifest);
 for(unsigned k=0;!e&&k<RELEASE_COUNT;k++){
  unsigned i=3;for(;i<s.header.count;i++)if(!strcmp(s.files[i].name,release_names[k]))break;if(i<s.header.count)continue;
  if(i==cap){unsigned next=cap*2;if(next<cap||next>0x7fffffffUL/sizeof *s.files){e=memFullErr;break;}RecoveryFile *p=(RecoveryFile*)NewPtr(next*sizeof *p);if(!p){e=memFullErr;break;}memcpy(p,s.files,cap*sizeof *p);memset(p+cap,0,(next-cap)*sizeof *p);DisposePtr((Ptr)s.files);s.files=p;cap=next;}
  strcpy(s.files[i].name,release_names[k]);s.files[i].data=update_be32(manifest+56+k*32+24);s.files[i].data_crc=update_be32(manifest+56+k*32+28);s.header.count++;
 }
 /* The1948 offline kit did not catalog its install-if-absent tools.
    Recognise that exact bundled image only; unknown older/manual copies stay. */
 if(!e){unsigned i=3;for(;i<s.header.count;i++)if(!strcmp(s.files[i].name,"hfsattr"))break;
  if(i==s.header.count){char old[256];strcpy(old,root_path);strcat(old,"bin:hfsattr");CInfoPBRec c;int found=info(old,&c);
   if(!found&&c.hFileInfo.ioFlLgLen==18916&&!c.hFileInfo.ioFlRLgLen&&c.hFileInfo.ioFlFndrInfo.fdType=='CLX1'&&c.hFileInfo.ioFlFndrInfo.fdCreator=='CLRC'){
    uint32_t crc;int verified=update_hash_fork(old,0,&crc,18916);if(!verified&&crc==0xe4a1c5edUL){if(i>=cap)e=memFullErr;else{strcpy(s.files[i].name,"hfsattr");s.files[i].data=18916;s.files[i].data_crc=crc;s.header.count++;}}
   }
  }
 }
 if(!e)e=recovery_prepare(&rotation,&s,helper,0);
 recovery_dispose(&s);if(e)errsay("Installer: cannot verify a coherent previous catalog. Existing files kept.\n");return e;
}
