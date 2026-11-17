/* Native transaction adapter. Journal and candidates are never interpreted as
 * arbitrary shell commands. Installed members are only the standard inventory
 * plus the exact application target. Caller stages/verifies content first. */
#ifndef CLARICE_RELEASE_NATIVE_H
#define CLARICE_RELEASE_NATIVE_H
#include "collection.h"
#include "release-manifest.h"
#include "native-files.h"
#include "open-guard.h"
#ifndef RELEASE_POLL
#define RELEASE_POLL() 0
#endif
#define RELEASE_JOURNAL_SIZE (556+40*(RELEASE_COUNT+1))
typedef struct {uint32_t id,modified,data,resource,created;} ReleaseIdentity;
typedef struct {
 char root[256],target[256];
 unsigned char identity[32];
 ReleaseIdentity old[RELEASE_COUNT+1],next[RELEASE_COUNT+1];
} ReleaseTransaction;
static void release_put32(unsigned char *p,uint32_t n){p[0]=n>>24;p[1]=n>>16;p[2]=n>>8;p[3]=n;}
static int release_path(const ReleaseTransaction *t,unsigned index,int slot,char out[256]){
 const char *name;char parent[256];size_t n;
 if(index>RELEASE_COUNT||slot<0||slot>2)return paramErr;
 if(index==RELEASE_COUNT){const char *leaf=strrchr(t->target,':');if(!leaf||!leaf[1])return bdNamErr;n=leaf-t->target+1;memcpy(parent,t->target,n);parent[n]=0;name=leaf+1;
  if(slot){name=slot==1?".release-core.new":".release-core.old";}
 }else {n=strlen(t->root);if(n+4>=sizeof parent)return bdNamErr;strcpy(parent,t->root);strcat(parent,"bin:");name=release_names[index];}
 if(index==RELEASE_COUNT||!slot){if(strlen(parent)+strlen(name)>255)return bdNamErr;strcpy(out,parent);strcat(out,name);}
 else {if(strlen(parent)+strlen(name)+9>255)return bdNamErr;strcpy(out,parent);strcat(out,".rl-");strcat(out,name);strcat(out,slot==1?".new":".old");}
 return 0;
}
static int release_state_path(const ReleaseTransaction *t,const char *leaf,char out[256]){
 if(strlen(t->root)+strlen(leaf)+12>255)return bdNamErr;
 strcpy(out,t->root);strcat(out,"etc:clarice:");strcat(out,leaf);return 0;
}
static int release_identity(const char *path,ReleaseIdentity *id){
 CInfoPBRec c;int e=update_info(path,&c);if(e)return e;if(c.hFileInfo.ioFlAttrib&16)return paramErr;
 id->id=c.hFileInfo.ioDirID;id->modified=c.hFileInfo.ioFlMdDat;id->data=c.hFileInfo.ioFlLgLen;id->resource=c.hFileInfo.ioFlRLgLen;id->created=c.hFileInfo.ioFlCrDat;return 0;
}
static int release_same(const ReleaseIdentity *a,const ReleaseIdentity *b){return a->id==b->id&&a->modified==b->modified&&a->data==b->data&&a->resource==b->resource&&a->created==b->created;}
static int release_old(void *ctx,unsigned i){return ((ReleaseTransaction*)ctx)->old[i].id!=0;}
static int release_identify(void *ctx,unsigned i,int slot){
 ReleaseTransaction *t=ctx;char path[256];ReleaseIdentity id;int e=release_path(t,i,slot,path);if(e)return e;
 e=release_identity(path,&id);if(e==fnfErr)return 0;if(e)return e;
 if(release_same(&id,&t->old[i]))return 1;if(release_same(&id,&t->next[i]))return 2;return paramErr;
}
static int release_move(void *ctx,unsigned i,int a,int b){
 ReleaseTransaction *t=ctx;char from[256],to[256];int e=release_path(t,i,a,from);if(!e)e=release_path(t,i,b,to);if(e)return e;
 /* All member renames are siblings. File Manager preserves both forks and all
    Finder metadata; never copy or delete an installed member here. */
 int busy=1;e=update_open_guard(from,&busy);if(e||busy)return e?e:fBsyErr;
 return update_rename(from,to);
}
static int release_flush(void *ctx){ReleaseTransaction *t=ctx;int e=update_flush(t->root);return e?e:update_flush(t->target);}
static int release_cancel(void *ctx){(void)ctx;return RELEASE_POLL();}
static void release_encode_identity(unsigned char *p,const ReleaseIdentity *id){release_put32(p,id->id);release_put32(p+4,id->modified);release_put32(p+8,id->data);release_put32(p+12,id->resource);release_put32(p+16,id->created);}
static void release_decode_identity(const unsigned char *p,ReleaseIdentity *id){id->id=update_be32(p);id->modified=update_be32(p+4);id->data=update_be32(p+8);id->resource=update_be32(p+12);id->created=update_be32(p+16);}
/* No in-place journal update. A distinct completion marker is written only
 * after every installed member has been checked and flushed. */
static int release_write_new(const char *path,const unsigned char *p,long size){
 short vol,ref=-1;int e=update_volume(path,&vol);if(e)return e;unsigned char n[256];update_pstr(n,path);
 e=HCreate(vol,0,n,'CLRC','DATA');if(e)return e;e=HOpen(vol,0,n,fsWrPerm,&ref);
 if(!e){long count=size;e=FSWrite(ref,&count,p);if(!e&&count!=size)e=ioErr;int close_error=FSClose(ref);if(!e)e=close_error;}
 /* A partial owned journal is deliberately retained: startup must fail closed. */
 return e?e:update_flush(path);
}
static int release_journal(void *ctx,int state){
 ReleaseTransaction *t=ctx;char path[256];int e;
 if(state==2){unsigned char done[36];memcpy(done,t->identity,32);UpdateCRC32 crc;update_crc32_init(&crc);update_crc32_bytes(&crc,done,32);release_put32(done+32,update_crc32_sum(&crc));
  e=release_state_path(t,"release.done",path);return e?e:release_write_new(path,done,sizeof done);}
 e=release_state_path(t,"release.pending",path);if(e)return e;
 if(!state){unsigned char n[256];short v;e=update_volume(path,&v);if(e)return e;update_pstr(n,path);e=HDelete(v,0,n);return e?e:update_flush(path);}
 unsigned char encoded[RELEASE_JOURNAL_SIZE];memset(encoded,0,sizeof encoded);memcpy(encoded,"CLRTX001",8);memcpy(encoded+8,t->root,strlen(t->root));memcpy(encoded+264,t->target,strlen(t->target));memcpy(encoded+520,t->identity,32);
 for(unsigned i=0;i<=RELEASE_COUNT;i++){release_encode_identity(encoded+552+i*40,&t->old[i]);release_encode_identity(encoded+572+i*40,&t->next[i]);}
 UpdateCRC32 crc;update_crc32_init(&crc);update_crc32_bytes(&crc,encoded,sizeof encoded-4);release_put32(encoded+sizeof encoded-4,update_crc32_sum(&crc));
 return release_write_new(path,encoded,sizeof encoded);
}
static CollectionOps release_ops(ReleaseTransaction *t){CollectionOps o={t,RELEASE_COUNT+1,release_old,release_identify,release_move,release_flush,release_journal,release_cancel};return o;}
#endif
